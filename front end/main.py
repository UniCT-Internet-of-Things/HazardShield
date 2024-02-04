from flask import Flask, render_template, url_for, request, redirect, send_from_directory, session, make_response
import pymongo as PyMongo
import json
import time

frontend_folder = './build/'
app = Flask(__name__,
            static_folder=frontend_folder+'_app',
            template_folder=frontend_folder,
            )