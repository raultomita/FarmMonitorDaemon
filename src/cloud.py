import threading
import queue
import logging
import os
import json
import hashlib
import hmac
import base64
import time
from datetime import datetime, timezone
from urllib.parse import quote
from email.utils import formatdate

import requests

import dataManager
import dispatcher

logger = logging.getLogger(__name__)

COSMOS_ENDPOINT = os.environ.get("COSMOS_ENDPOINT", "").rstrip("/")
COSMOS_KEY      = os.environ.get("COSMOS_KEY", "")
COSMOS_DATABASE = os.environ.get("COSMOS_DATABASE", "farmmonitor")

CONTAINER_STATES   = "device-states"
CONTAINER_HEARTBEATS = "heartbeats"
CONTAINER_COMMANDS = "commands"

POLL_INTERVAL_SECONDS = 30
REQUEST_TIMEOUT_SECONDS = 5

cloud_queue = queue.Queue()

def enqueueCloudEvent(event_type, payload):
    """Push an event onto the cloud queue (non-blocking)."""
    cloud_queue.put((event_type, payload))


# ---------------------------------------------------------------------------
# Cosmos DB REST authentication helpers
# ---------------------------------------------------------------------------

def _auth_header(method: str, resource_type: str, resource_link: str, date_str: str) -> str:
    """Generate the Authorization header value for Cosmos DB REST API."""
    string_to_sign = "\n".join([
        method.lower(),
        resource_type.lower(),
        resource_link,
        date_str.lower(),
        "",
        ""
    ])
    key = base64.b64decode(COSMOS_KEY)
    signature = base64.b64encode(
        hmac.new(key, string_to_sign.encode("utf-8"), hashlib.sha256).digest()
    ).decode("utf-8")
    return quote("type=master&ver=1.0&sig=" + signature)


def _headers(method: str, resource_type: str, resource_link: str) -> dict:
    date_str = formatdate(usegmt=True)
    return {
        "Authorization": _auth_header(method, resource_type, resource_link, date_str),
        "x-ms-date": date_str,
        "x-ms-version": "2018-12-31",
        "Content-Type": "application/json",
    }


def _doc_link(container: str, doc_id: str = "") -> str:
    base = f"dbs/{COSMOS_DATABASE}/colls/{container}"
    return f"{base}/docs/{doc_id}" if doc_id else f"{base}/docs"


def _base_url(container: str, doc_id: str = "") -> str:
    base = f"{COSMOS_ENDPOINT}/dbs/{COSMOS_DATABASE}/colls/{container}"
    return f"{base}/docs/{doc_id}" if doc_id else f"{base}/docs"


# ---------------------------------------------------------------------------
# Cosmos DB operations
# ---------------------------------------------------------------------------

def _upsert_document(container: str, document: dict, partition_value: str):
    """Upsert a document into a Cosmos DB container. Logs and drops on failure."""
    resource_link = f"dbs/{COSMOS_DATABASE}/colls/{container}"
    headers = _headers("POST", "docs", resource_link)
    headers["x-ms-documentdb-is-upsert"] = "true"
    headers["x-ms-documentdb-partitionkey"] = json.dumps([partition_value])

    try:
        resp = requests.post(
            _base_url(container),
            headers=headers,
            json=document,
            timeout=REQUEST_TIMEOUT_SECONDS,
        )
        if resp.status_code not in (200, 201):
            logger.warning("Cosmos upsert failed (%d): %s", resp.status_code, resp.text[:200])
    except Exception as exc:
        logger.warning("Cosmos upsert error: %s", exc)


def _patch_document_status(container: str, doc_id: str, partition_value: str, status: str):
    """Patch the status field of a document (replace). Logs and drops on failure."""
    resource_link = _doc_link(container, doc_id)
    headers = _headers("GET", "docs", resource_link)
    headers["x-ms-documentdb-partitionkey"] = json.dumps([partition_value])

    try:
        resp = requests.get(
            _base_url(container, doc_id),
            headers=headers,
            timeout=REQUEST_TIMEOUT_SECONDS,
        )
        if resp.status_code != 200:
            logger.warning("Cosmos fetch doc failed (%d): %s", resp.status_code, resp.text[:200])
            return

        doc = resp.json()
        doc["status"] = status

        headers_put = _headers("PUT", "docs", resource_link)
        headers_put["x-ms-documentdb-partitionkey"] = json.dumps([partition_value])
        headers_put["If-Match"] = doc.get("_etag", "")

        resp2 = requests.put(
            _base_url(container, doc_id),
            headers=headers_put,
            json=doc,
            timeout=REQUEST_TIMEOUT_SECONDS,
        )
        if resp2.status_code not in (200, 201):
            logger.warning("Cosmos patch failed (%d): %s", resp2.status_code, resp2.text[:200])
    except Exception as exc:
        logger.warning("Cosmos patch error: %s", exc)


def _query_pending_commands(hostname: str) -> list:
    """Return all pending command documents targeting this host."""
    resource_link = f"dbs/{COSMOS_DATABASE}/colls/{CONTAINER_COMMANDS}"
    headers = _headers("POST", "docs", resource_link)
    headers["x-ms-documentdb-isquery"] = "true"
    headers["x-ms-documentdb-query-enablecrosspartition"] = "false"
    headers["x-ms-documentdb-partitionkey"] = json.dumps([hostname])
    headers["Content-Type"] = "application/query+json"

    query_body = {
        "query": "SELECT * FROM c WHERE c.targetHost = @host AND c.status = 'pending'",
        "parameters": [{"name": "@host", "value": hostname}],
    }

    try:
        resp = requests.post(
            _base_url(CONTAINER_COMMANDS),
            headers=headers,
            json=query_body,
            timeout=REQUEST_TIMEOUT_SECONDS,
        )
        if resp.status_code == 200:
            return resp.json().get("Documents", [])
        else:
            logger.warning("Cosmos query failed (%d): %s", resp.status_code, resp.text[:200])
    except Exception as exc:
        logger.warning("Cosmos query error: %s", exc)

    return []


# ---------------------------------------------------------------------------
# Cloud manager thread
# ---------------------------------------------------------------------------

class CloudManagerThread(threading.Thread):
    def __init__(self):
        super().__init__()
        self.daemon = True
        self._last_poll = 0.0

    def run(self):
        if not COSMOS_ENDPOINT or not COSMOS_KEY:
            logger.warning("CloudManagerThread: COSMOS_ENDPOINT or COSMOS_KEY not set — cloud sync disabled.")
            return

        logger.info("CloudManagerThread started (endpoint=%s, db=%s)", COSMOS_ENDPOINT, COSMOS_DATABASE)

        while True:
            # --- Drain outgoing event queue ---
            while not cloud_queue.empty():
                try:
                    event_type, payload = cloud_queue.get_nowait()
                    self._handle_outgoing(event_type, payload)
                except queue.Empty:
                    break

            # --- Poll for incoming cloud commands every 30 s ---
            now = time.monotonic()
            if now - self._last_poll >= POLL_INTERVAL_SECONDS:
                self._last_poll = now
                self._poll_commands()

            time.sleep(1)

    def _handle_outgoing(self, event_type: str, payload):
        if event_type == "state":
            doc = json.loads(payload) if isinstance(payload, str) else dict(payload)
            doc["hostname"] = dataManager.hostName
            # Use composite id so upsert is idempotent per device per host
            doc["id"] = f"{dataManager.hostName}_{doc.get('id', 'unknown')}"
            _upsert_document(CONTAINER_STATES, doc, dataManager.hostName)
            logger.debug("Cloud state upserted for %s", doc.get("id"))

        elif event_type == "heartbeat":
            doc = {
                "id": dataManager.hostName,
                "hostname": dataManager.hostName,
                "lastSeen": payload,
            }
            _upsert_document(CONTAINER_HEARTBEATS, doc, dataManager.hostName)
            logger.debug("Cloud heartbeat upserted for %s", dataManager.hostName)

    def _poll_commands(self):
        logger.debug("Polling Cosmos DB for pending commands")
        pending = _query_pending_commands(dataManager.hostName)
        logger.debug("Found %d pending cloud command(s)", len(pending))

        for cmd_doc in pending:
            doc_id    = cmd_doc.get("id", "")
            command   = cmd_doc.get("command", "")
            timestamp = cmd_doc.get("timestamp", "")

            if not command:
                _patch_document_status(CONTAINER_COMMANDS, doc_id, dataManager.hostName, "processed")
                continue

            # Stale-command check: skip if command timestamp predates last known device state
            device_id = command.split(":")[0] if ":" in command else command
            if _is_stale(device_id, timestamp):
                logger.info("Skipping stale cloud command '%s' (timestamp=%s)", command, timestamp)
                _patch_document_status(CONTAINER_COMMANDS, doc_id, dataManager.hostName, "processed")
                continue

            logger.info("Dispatching cloud command: %s", command)
            dispatcher.enqueueCommand(command)
            _patch_document_status(CONTAINER_COMMANDS, doc_id, dataManager.hostName, "processed")


def _is_stale(device_id: str, command_timestamp_str: str) -> bool:
    """Return True if the command timestamp is older than the device's last state change."""
    if not command_timestamp_str:
        return False

    last_state_time = dataManager.device_state_times.get(device_id)
    if last_state_time is None:
        return False

    try:
        cmd_time = datetime.fromisoformat(command_timestamp_str.replace("Z", "+00:00"))
        # Ensure both are offset-aware for comparison
        if cmd_time.tzinfo is None:
            cmd_time = cmd_time.replace(tzinfo=timezone.utc)
        if last_state_time.tzinfo is None:
            last_state_time = last_state_time.replace(tzinfo=timezone.utc)
        return cmd_time < last_state_time
    except ValueError:
        logger.warning("Cannot parse command timestamp: %s", command_timestamp_str)
        return False
