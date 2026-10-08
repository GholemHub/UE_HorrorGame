"""Run read-only Blueprint and asset regressions in a single Unreal commandlet."""
from pathlib import Path
import runpy
import unreal

assert '-run=pythonscript' in unreal.SystemLibrary.get_command_line().lower()
scripts = Path(unreal.Paths.project_dir()) / 'Scripts'
for name in (
    'test_optimization_blueprints.py',
    'test_audio_voice_assets.py',
    'test_death_timeline_blueprint.py',
    'test_pickup_ownership.py',
    'test_session_blueprints.py',
    'test_ritual_candles.py',
    'test_table_ritual.py',
    'test_paint_cube.py',
):
    unreal.log('HRONO_REGRESSION ' + name)
    runpy.run_path(str(scripts / name), run_name='__main__')
