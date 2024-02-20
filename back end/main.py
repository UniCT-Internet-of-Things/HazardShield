from flask import Flask, render_template, url_for, request, redirect, send_from_directory, session, make_response
import pymongo as PyMongo
import json
import time
import requests
import threading
frontend_folder = './build/'
app = Flask(__name__,
            static_folder=frontend_folder+'_app',
            template_folder=frontend_folder,
            )
global gateway_ip
gateway_ip=""

#create a thread

def send_request():
    while True:
        #send request to the gateway
        if gateway_ip != "":    
            print("Sending request to the gateway")
            response = requests.post("http://"+gateway_ip+":80/receive_data", data = {'datas': 'data', 'values': {"test":"2","test2":"4"}})   
            print(response)
            time.sleep(5)

thread = threading.Thread(target=send_request)

@app.route('/get_data', methods=['POST'])
def ricevi_dati():

    print(request)
    #stampa tutta la struttura della richiesta
    print(request.data)
    return 'Dati ricevuti con successo!'

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
    