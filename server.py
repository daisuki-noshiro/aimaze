from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
import json
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parent
DATA_DIR = ROOT / "data"
EXE_PATH = ROOT / "build" / "MazeAI.exe"
ALGORITHMS = {"greedy", "astar", "dijkstra", "bfs", "divide"}


class MazeHandler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(ROOT), **kwargs)

    def do_GET(self):
        if self.path == "/":
            self.send_response(302)
            self.send_header("Location", "/output/visualization.html")
            self.end_headers()
            return

        if self.path == "/api/maps":
            maps = sorted(path.name for path in DATA_DIR.glob("*.json"))
            self.send_json({"maps": maps, "algorithms": sorted(ALGORITHMS)})
            return

        super().do_GET()

    def do_POST(self):
        if self.path != "/api/run":
            self.send_error(404, "Not found")
            return

        try:
            length = int(self.headers.get("Content-Length", "0"))
            payload = json.loads(self.rfile.read(length) or b"{}")
            map_name = str(payload.get("map", ""))
            algorithm = str(payload.get("algorithm", ""))

            if Path(map_name).name != map_name or not map_name.endswith(".json"):
                raise ValueError("Invalid map file")
            if algorithm not in ALGORITHMS:
                raise ValueError("Invalid algorithm")

            map_path = DATA_DIR / map_name
            if not map_path.is_file():
                raise ValueError("Map file not found")
            if not EXE_PATH.is_file():
                raise ValueError("MazeAI.exe not found. Build the project first.")

            completed = subprocess.run(
                [str(EXE_PATH), f"data/{map_name}", algorithm],
                cwd=str(ROOT),
                capture_output=True,
                text=True,
                timeout=30,
            )

            if completed.returncode != 0:
                self.send_json(
                    {
                        "ok": False,
                        "error": completed.stderr.strip() or completed.stdout.strip(),
                    },
                    status=500,
                )
                return

            result_path = ROOT / "output" / "result.json"
            result = {}
            if result_path.is_file():
                result = json.loads(result_path.read_text(encoding="utf-8"))

            self.send_json({"ok": True, "result": result})
        except subprocess.TimeoutExpired:
            self.send_json({"ok": False, "error": "Run timed out after 30 seconds"}, status=500)
        except Exception as exc:
            self.send_json({"ok": False, "error": str(exc)}, status=400)

    def send_json(self, data, status=200):
        body = json.dumps(data, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)


if __name__ == "__main__":
    server = ThreadingHTTPServer(("127.0.0.1", 8000), MazeHandler)
    print("Maze visualizer server: http://127.0.0.1:8000/output/visualization.html")
    server.serve_forever()
