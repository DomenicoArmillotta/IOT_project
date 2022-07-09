import pymysql.cursors

# Connect to the database

'''
    Class Database: open a connection through mysql database, to database datacollector containing the coapsensors 
    table (with information about user entering and leaving) and the mqttsensors (with information about weather)
    An instance of Database is assigned both to mote resource (each single lock sensor) and to the mqtt_collector
'''
class Database:
    connection = None

    def __init__(self):
        print("Instantiating!")

    def connect_dbs(self):

        if self.connection is not None:
            return self.connection
        else:
            self.connection = pymysql.connect(host='localhost',
                 user='root',
                 password='PASSWORD',
                 database='datacollector',
                 cursorclass=pymysql.cursors.DictCursor)
            return self.connection
