from flask import Flask, jsonify, request
import socket, subprocess, sys

app = Flask(__name__)

SERVER_EXE = r"C:\Users\sokys\Desktop\MYFOLDEROFSTUFF\GameProjects\StellarStrategem\StellarStrategemBuild\WindowsServer\StellarStratagemServer.exe"
MAP_NAME = "L_GameServer"
PUBLIC_IP = "192.168.0.192"

# Stores active games: { game_code : { "port": 7777, "pid": 1234 } }
ACTIVE_GAMES = {}


def find_free_port():
    import socket
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind(('', 0))
    addr, port = s.getsockname()
    s.close()
    return port


def start_server_process(port, game_code):
    args = [
        SERVER_EXE,
        f"{MAP_NAME}?listen",
        f"-port={port}",
        f"-gamecode={game_code}",
        "-log"
    ]

    kwargs = {}
    if sys.platform == "win32":
        CREATE_NO_WINDOW = 0x08000000
        kwargs.update(creationflags=CREATE_NO_WINDOW)

    proc = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, **kwargs)
    return proc.pid


@app.route("/create-game", methods=["POST"])
def create_game():
    data = request.get_json(silent=True) or {}
    game_code = data.get("game_code")

    if not game_code:
        return jsonify({"error": "Game name is required."}), 400

    if game_code in ACTIVE_GAMES:
        return jsonify({"error": "Game with given name already exists."}), 400

    port = find_free_port()
    pid = start_server_process(port, game_code)

    ACTIVE_GAMES[game_code] = {
        "port": port,
        "pid": pid
    }

    return jsonify({
        "ip": PUBLIC_IP,
        "port": port
    })


@app.route("/join-game", methods=["POST"])
def join_game():
    data = request.get_json(silent=True) or {}
    game_code = data.get("game_code")

    if not game_code:
        return jsonify({"error": "Game name is required."}), 400

    game = ACTIVE_GAMES.get(game_code)
    if not game:
        return jsonify({"error": "Game with given name does not exist."}), 404

    return jsonify({
        "game_code": game_code,
        "ip": PUBLIC_IP,
        "port": game["port"]
    })


@app.route("/shutdown-game", methods=["POST"])
def shutdown_game():
    data = request.get_json(silent=True) or {}
    game_code = data.get("game_code")

    if not game_code:
        return jsonify({"error": "Game name is required."}), 400

    game = ACTIVE_GAMES.pop(game_code, None)
    if not game:
        return jsonify({"error": "Game with given name not found."}), 404

    return jsonify({"status": "removed"})


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=7777)
