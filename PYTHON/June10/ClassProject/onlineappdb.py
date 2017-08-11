import os, django, sys
os.environ.setdefault("DJANGO_SETTINGS_MODULE", "ClassProject.settings")
# sys.path.append(os.getcwd())

from django.core.wsgi import get_wsgi_application
application = get_wsgi_application()

import click
from ClassProject.settings import DATABASES
import MySQLdb
from openpyxl import Workbook
from openpyxl import load_workbook
from onlineapp.models import College, Student, MockTest1
from bs4 import BeautifulSoup



@click.group()
def onlineappdb():
    pass

database = DATABASES['default']
host = 'localhost'
user = database['USER']
password = database['PASSWORD']
db_name = database['NAME']


@onlineappdb.command()
def createdb():
    '''Creates the database'''
    conn = None
    try:
        conn = MySQLdb.connect(host=host, user=user, passwd=password)
        conn.query("CREATE DATABASE IF NOT EXISTS {}".format(db_name))
        conn.select_db(db_name)

        click.echo("Database successfully created")
    except MySQLdb.Error as e:
        click.echo(e)
    finally:
        if conn:
            conn.close()


@onlineappdb.command()
def dropdb():
    '''Drops the database'''
    conn = None
    try:
        conn = MySQLdb.connect(host=host, user=user, passwd=password)
        conn.query("DROP DATABASE {}".format(db_name))
        click.echo("Dropped Database")
    except MySQLdb.Error as e:
        click.echo(e)
    finally:
        if conn:
            conn.close()


def get_sheet_data(sheet):
    data = []
    row_count = len(sheet.rows)

    for row in range(row_count):
        row_data = [str(cell.value) for cell in sheet.rows[row]]
        data.append(row_data)

    return data


def extract_data_from_excel(src_excel):

    src_workbook = load_workbook(filename=src_excel, data_only=True)

    colleges = get_sheet_data(src_workbook.get_sheet_by_name('Colleges'))[1:]
    colleges = [College(name=c[0], acronym=c[1], location=c[2], contact=c[3]) for c in colleges]
    College.objects.bulk_create(colleges)
    students = get_sheet_data(src_workbook.get_sheet_by_name('Current'))[1:]
    dropped_students = get_sheet_data(src_workbook.get_sheet_by_name('Deletions'))[1:]
    student_objects = []

    for s in students:
        college_name = s[1]
        student_objects.append(Student(
            name=s[0],
            college=College.objects.get(acronym=college_name),
            email=s[2],
            db_folder=str(s[3].lower()),
            dob=None
        ))

    Student.objects.bulk_create(student_objects)

    student_objects = []

    for s in dropped_students:
        college_name = s[1]
        try:
            c = College.objects.get(acronym=college_name)
            student_objects.append(Student(
                name=s[0],
                college=c,
                email=s[2],
                db_folder=str(s[3].lower()),
                dob=None,
                dropped_out=True
            ))
        except College.DoesNotExist:
            print "corrupted record: ", s

    Student.objects.bulk_create(student_objects)

    click.echo("Successfully extracted")


def get_string(cell):
    anchor_tag = cell.a
    string = str(anchor_tag.contents[0])
    return string


def transform_to_types(row):
    row[0] = get_string(row[0])
    for i in range(1, 6):
        # convert contents to number
        row[i] = int(row[i].contents[0].strip())


def extract_data_from_html(source_html):

    data = []
    with open(source_html) as html_doc:
        html_text = BeautifulSoup(html_doc, "html.parser")
        table = html_text.findAll('table')[0]
        rows = table.findAll('tr')[1:]

        for row in rows:
            record = row.findAll('td')[1:]
            transform_to_types(record)
            try:
                name = Student.objects.get(db_folder=record[0].split('_')[2])
                data.append(MockTest1(
                    student=name,
                    problem1=record[1],
                    problem2=record[2],
                    problem3=record[3],
                    problem4=record[4],
                    total=record[5]
                ))
            except Student.DoesNotExist as e:
                print "corrupted record: ", record

    MockTest1.objects.bulk_create(data)


@onlineappdb.command()
@click.argument("src_excel", nargs=1)
@click.argument("src_html", nargs=1)
def populatedb(src_excel, src_html):
    '''Populates the database with the data from excel and html files'''
    extract_data_from_excel(src_excel)
    extract_data_from_html(src_html)


@onlineappdb.command()
def cleardata():
    '''Deletes all data in the tables'''
    MockTest1.objects.all().delete()
    Student.objects.all().delete()
    College.objects.all().delete()
    click.echo("Successfully cleared all the data")


if __name__ == '__main__':
    onlineappdb()
