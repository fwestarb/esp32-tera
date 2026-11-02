"""Local support export: deliberately exclude raw status, errors and identifiers."""
import json
from pathlib import Path

KNOWN_FIRMWARE = {'0.5.0', '0.5.1', '1.0.0-alpha.1', '1.0.0-alpha.2'}

def build_report(bridge):
    status = bridge.last_status
    version = status.get('version')
    return {
        'schemaVersion': 1,
        'product': 'esp32-tera',
        'companion': {
            'connected': bool(bridge.connection and bridge.connection.is_open),
            'ready': bool(bridge.ready),
            'busy': bool(bridge.busy),
            'actionsEnabled': bool(bridge.actions),
            'speechEnabled': bool(bridge.speech.enabled),
        },
        'device': {
            'firmware': version if isinstance(version, str) and version in KNOWN_FIRMWARE else None,
            'wifiConnected': status.get('wifi') if type(status.get('wifi')) is bool else None,
        },
    }

def export_report(bridge, destination):
    report = build_report(bridge)
    Path(destination).write_text(json.dumps(report, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
    return report
