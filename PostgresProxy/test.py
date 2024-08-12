#!/usr/bin/python3

import psycopg2
from psycopg2 import Error
import threading
import time


def table_exists(cursor, name):
	cursor.execute("select exists(select relname from pg_class where relname='"+name+"')")
	exists = cursor.fetchone()[0]
	return exists

def create_table(cursor):
	create_tbl_query = """CREATE TABLE IF NOT EXISTS accounts (
		id serial PRIMARY KEY,
		name VARCHAR ( 50 ) UNIQUE NOT NULL,
		password VARCHAR ( 50 ) NOT NULL,
		email VARCHAR ( 255 ) UNIQUE NOT NULL
	);"""

	print("Creating table")
	cursor.execute(create_tbl_query)

def fill_table(cursor):
	insert_query = """INSERT INTO accounts(id, name, password, email) VALUES
											(1, 'user1', 'pwd1', 'user1@test.com'),
											(2, 'user2', 'pwd2', 'user2@test.com'),
											(3, 'user3', 'pwd3', 'user3@test.com');
	"""
	cursor.execute(insert_query)

def show_table(cursor):
	select_query = "select * from accounts"
	cursor.execute(select_query)
	records = cursor.fetchall()
	for row in records:
		print("ID: ", row[0])
		print("NAME: ", row[1])
		print("PWD: ", row[2])
		print("EMAIL: ", row[3], "\n")

def test_db():
	try:
		# Подключиться к существующей базе данных
		connection = psycopg2.connect(user="andrey",
										password="qwerty",
										host="127.0.0.1",
										port="6776",#5432
										database="test",
										sslmode="disable")
		print("Connected")
		cursor = connection.cursor()
		if table_exists(cursor, "accounts") != True:
			create_table(cursor)
			fill_table(cursor)
			connection.commit()

		show_table(cursor)
	except (Exception, Error) as error:
		print("Error: ", error)
	finally:
		if connection:
			cursor.close()
			connection.close()
			print("Connection closed")

def main():
	threads = list()
	for index in range(100):
		x = threading.Thread(target=test_db)
		threads.append(x)
		x.start()
		time.sleep(10/1000)
	
	# Wait for completion
	for t in threads:
		t.join()

if __name__ == "__main__":
	main()
