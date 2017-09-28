from flask import Flask, render_template
from flask_socketio import SocketIO

app = Flask(__name__)
socketio = SocketIO(app)

@socketio.on('message')
def handle_message(message):
    print('received message: ' + message)

if __name__ == '__main__':
    socketio.run(app, host='192.168.1.101', port=8000)