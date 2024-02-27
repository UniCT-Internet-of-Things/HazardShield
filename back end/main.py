from flask import Flask, render_template, url_for, request, redirect, send_from_directory, session, make_response
import pymongo as PyMongo
from flask_sock import Sock
import json
import time
import requests
import threading
from flask_cors import CORS

frontend_folder = './build/'
app = Flask(__name__,
            static_folder=frontend_folder+'_app',
            template_folder=frontend_folder,
            )
sock = Sock(app)
global gateway_ip
gateway_ip=""

CORS(app)
#create a thread

mongo = PyMongo.MongoClient('localhost:27017', 27017)

global ws
ws=None

def send_request():

    while True:
        global ws
        if(ws is not None):
            print("sending request")
            ws.send('Hello, world!')
        time.sleep(20)

thread = threading.Thread(target=send_request)

@app.route('/get_data', methods=['POST'])
def ricevi_dati():

    print(request)
    #stampa tutta la struttura della richiesta
    print(request.data)
    return 'Dati ricevuti con successo!'

@sock.route('/get_all')
def echo(websocket):
    
    while True:
        global ws
        ws=websocket
        data = ws.receive()
        print(data)

@app.route('/post_data', methods=['POST'])
def post_data():
    print(request)
    #stampa tutta la struttura della richiesta
    print(request.data)

    #convert data from byte to string
    data = request.data.decode('utf-8')
    print(data)
    #convert data from string to dictionary
    data = json.loads(data)
    print(data)
    Worker=mongo.workers.data
    

    for key in data:
        data_to_insert={}
        for key2 in data[key]:

            data_to_insert["mac_dispositivo"]=key2
            data_to_insert["timestamp"]=time.time()
            data_to_insert["sensor_data"]=data[key][key2]
            data_to_insert["sensor_id"]=key

            Worker.insert_one(data_to_insert)

    return 'Dati ricevuti con successo!'    
    
@app.route('/put_worker', methods=['POST'])
def put_worker():
    print(request)
    #stampa tutta la struttura della richiesta
    print(request.data)
    data = request.data.decode('utf-8')
    print(data)
    #convert data from string to dictionary
    data = json.loads(data)
    user={
        "nome":data["nome"],
        "cognome":data["cognome"],
        "eta":data["eta"],
        "task":data["task"],
        "info":data["info"]
    }
    personal=mongo.workers.personal_data

    personal.insert_one(user)

    return 'Dati ricevuti con successo!'

@app.route('/get_all_names', methods=['GET'])
def get_all_names():

    print(request)
    
    personal=mongo.workers.personal_data

    cursor = personal.find({})
    ritorno={}
    i=0
    for document in cursor:
        del document["_id"]
        print(document)
        ritorno[i]=document
        i+=1

    return ritorno



@app.route('/register_ip', methods=['POST'])
def register_ip():

    print(request)
    #stampa tutta la struttura della richiesta
    print(request.data)

    #convert data from byte to string
    data = request.data.decode('utf-8')
    print(data)
    #convert data from string to dictionary
    data = json.loads(data)
    print(data)
    #get the ip of the sender
    global gateway_ip
    gateway_ip = request.remote_addr
    print(gateway_ip)
    return 'Dati ricevuti con successo!'

if __name__ == '__main__':
    thread.start()
    app.run(debug=True,host="0.0.0.0")
    