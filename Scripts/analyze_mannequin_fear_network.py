"""Validate recorded two-process evidence; no game/editor state changes."""
import datetime as dt
import json
from pathlib import Path

root = Path(__file__).resolve().parents[1] / 'Saved/Tests/Mannequin/Fear/UnconditionalNetwork'
logs = {role: (root / f'{role}.log').read_text(encoding='utf-8', errors='replace')
        for role in ('Host', 'Client', 'Reconnect')}
samples = {role: [dict(field.split('=', 1) for field in line.split('[FearProbe] ')[1].split())
                 for line in text.splitlines() if '[FearProbe] action=' in line]
           for role, text in logs.items()}
checks = {}
for role, records in samples.items():
    active = [r for r in records if r['active'] == '1']
    checks[f'{role}: expected network role'] = bool(records) and all(
        r['net'] == ('2' if role == 'Host' else '3') for r in records)
    checks[f'{role}: active fear received'] = bool(active)
    checks[f'{role}: fixed authority position'] = bool(active) and all(
        abs(float(r['x'])) < 0.01 and abs(float(r['y'])) < 0.01
        and abs(float(r['z']) - 50000) < 0.01 for r in active)
    poses = [r for r in active if float(r['sampled']) >= 0]
    checks[f'{role}: rendered sequence follows clock/holds final frame'] = bool(poses) and all(
        abs(float(r['sampled']) - (20.0 if r['holding'] == '1' else min(20.0, 4 * float(r['elapsed'])))) < 0.08 for r in poses)
    checks[f'{role}: latency/loss enabled'] = ('PktLag set to 100' in logs[role]
                                             and 'PktLoss set to 2' in logs[role])

for role, records in samples.items():
    active = [r for r in records if r['active'] == '1']
    checks[f'{role}: gaze never pauses fear'] = bool(active) and all(r['paused'] == '0' for r in active)
    checks[f'{role}: no capsule or body collision during fear'] = bool(active) and all(
        r['capsule'] == '0' and r['body'] == '0' for r in active)
    checks[f'{role}: holds final frame after manifestation'] = any(
        r['holding'] == '1' and abs(float(r['sampled']) - 20) < 0.01 for r in active)
checks['Host intro advances while watching'] = any(
    r['active'] == '1' and r['holding'] == '0' and 1 < float(r['sampled']) < 20 for r in samples['Host'])
checks['Remote receives restored collision after danger'] = any(
    r['action'] == 'status' and r['active'] == '0' and r['capsule'] != '0'
    for r in samples['Reconnect'])
checks['Reconnect restores an existing fear pose, without replay'] = any(
    r['active'] == '1' and float(r['elapsed']) > 20 and abs(float(r['sampled']) - 20) < 0.01
    for r in samples['Reconnect'])
checks['Host deactivation clears fear'] = any(r['action'] == 'cleanup' and r['active'] == '0'
                                            for r in samples['Host'])

def event_time(fragment):
    line = next(line for line in logs['Host'].splitlines() if fragment in line)
    return dt.datetime.strptime(line[1:24], '%Y.%m.%d-%H.%M.%S:%f')

lead_seconds = (event_time('Hunt state Anticipation -> Manifestation')
                - event_time('Hunt state None -> Anticipation')).total_seconds()
checks['Actual manifestation waits five seconds'] = 4.95 <= lead_seconds <= 5.15
result = {'passed': all(checks.values()), 'checks': checks, 'lead_seconds': lead_seconds,
          'samples': samples,
          'limitations': ['Headless: no final-art visual acceptance.',
                          'Death, travel and final-art player walkthrough still need manual coverage.']}
(root / 'analysis.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
print(json.dumps({'passed': result['passed'], 'lead_seconds': lead_seconds,
                  'checks': len(checks), 'failed': [k for k, v in checks.items() if not v]}))
assert result['passed'], 'See Network/analysis.json'
