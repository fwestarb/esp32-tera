"""Install Windows companion dependencies in a relocatable project environment."""
from pathlib import Path
import subprocess
import sys
import venv

ROOT=Path(__file__).resolve().parents[1]
def main():
    if sys.version_info[:2]!=(3,12):raise SystemExit('Use Python 3.12 de python.org para esta alpha.')
    environment=ROOT/'.venv-pc';python=environment/'Scripts/python.exe'
    if not python.exists():venv.EnvBuilder(with_pip=True).create(environment)
    subprocess.run([str(python),'-m','pip','install','-r',str(ROOT/'tools/requirements.txt')],check=True)
    print('Aplicativo instalado. Abra configurar.cmd.')
if __name__=='__main__':main()
