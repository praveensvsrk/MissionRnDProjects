import os, django, sys
os.environ.setdefault("DJANGO_SETTINGS_MODULE", "ClassProject.settings")
from django.core.wsgi import get_wsgi_application
application = get_wsgi_application()
from django.core.management.base import BaseCommand
import re
import MySQLdb
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.image import MIMEImage
from string import Template
from onlineapp.models import *
import numpy as np
import matplotlib.pyplot as plt



def is_email_valid(email):
    if re.match("^[_a-z0-9-]+(\.[_a-z0-9-]+)*@[a-z0-9-]+(\.[a-z0-9-]+)*(\.[a-z]{2,4})$", email) is not None:
        return True
    return False


def execute_query(cursor, query):
    try:
        cursor.execute(query)
        results = cursor.fetchall()
    except MySQLdb.Error as e:
        raise e
    return results


def get_email_template():
    str = "Hello\r\n\r\n" \
          "This is an auto generated mail from Mission R&D. The below marks is your students' performance in the test," \
          " along with other colleges' performance for comparison\r\n\r\n" \
          "1. Students performance:(Sorted by Marks)\r\n\r\n${STUDENT_LIST}\r\n\r\n" \
          "2. Summary:(Sorted by Average)\r\n\r\n${SUMMARY}\r\n\r\n" \
          "Thank you\r\n"
    return Template(str)


def send_email(from_address, to_address, email_text):
    try:
        server = smtplib.SMTP('smtp.gmail.com', 587)
        server.ehlo()
        server.starttls()
        server.ehlo()
        server.login("sk.sk.praveen@gmail.com", "praveen147890")
        server.sendmail(from_address, to_address, email_text)
        server.close()
    except smtplib.SMTPException as e:
        print e
        print 'Something went wrong...'


def college_report(college):

    students = Student.objects.filter(college__acronym=college).order_by('-mocktest1__total')
    student_list = ["{:>40}  {:>5}".format("Student", "Marks")]
    student_list.append("")
    for s in students:
        try:
            student_list.append("{:>40}  {:>5}".format(s.name, s.mocktest1.total))
        except:
            student_list.append("{:>40}  {:>5}".format(s.name, 0))

    summary = ["{:>10}  {:>5}  {:>5}  {:>10}  {:>5}".format("College", "Count", "Min", "Average", "Max")]
    summary.append("")

    #
    # for s in students:
    #     summary.append("{:>10}  {:>5}  {:>5}  {:>10}  {:>5}".format(college[0], count, min, avg, max))


    colleges = College.objects.all()

    min_marks = []
    avg_marks = []
    max_marks = []

    for c in colleges:
        marks = MockTest1.objects.filter(student__college__acronym=c.acronym).values_list('total')

        min_marks.append(int(min(marks)[0]))
        avg_marks.append(sum([res[0] for res in marks])/len(marks))
        max_marks.append(int(max(marks)[0]))

        summary.append("{:>10}  {:>5}  {:>5}  {:>10}  {:>5}".format(c.acronym, len(marks), min_marks[-1], avg_marks[-1],
                                                                    max_marks[-1]))

    plt.rcParams["figure.figsize"] = [10, 6]
    fig, ax = plt.subplots()

    index = np.arange(colleges.count())
    bar_width = 0.2

    colleges = map(lambda x: x[0][:3], list(colleges.values_list('acronym')))
    opacity = 0.7
    error_config = {'ecolor': '0.3'}

    min_bar = plt.bar(index, min_marks, bar_width,
                     alpha=opacity,
                     color='r',
                     yerr=None,
                     error_kw=error_config,
                     label='Min')

    avg_bar = plt.bar(index + bar_width, avg_marks, bar_width,
                     alpha=opacity,
                     color='y',
                     yerr=None,
                     error_kw=error_config,
                     label='Avg')

    max_bar = plt.bar(index + bar_width * 2, max_marks, bar_width,
                     alpha=opacity,
                     color='g',
                     yerr=None,
                     error_kw=error_config,
                     label='Max')

    plt.xlabel('Colleges')
    plt.ylabel('Scores')
    plt.title('Scores by Colleges')
    plt.xticks(index + bar_width, colleges)
    plt.legend()

    plt.tight_layout()
    plt.savefig('foo.png')


    msg = MIMEMultipart()
    msg['Subject'] = 'College marks'
    msg['From'] = ''
    msg['To'] = ''

    email_template = get_email_template()
    email_text = email_template.substitute(STUDENT_LIST='\r\n'.join(student_list),
                                           SUMMARY='\r\n'.join(summary),)

    text = MIMEText(email_text)
    msg.attach(text)
    image = MIMEImage(open('foo.png', 'rb').read(), name='foo.png')
    msg.attach(image)

    send_email("sk.sk.praveen@gmail.com", "sk.sk.praveen@gmail.com", msg.as_string())



class Command(BaseCommand):
    help = "Sends an email with marks"

    def add_arguments(self, parser):
        # Positional arguments
        parser.add_argument('college_name', nargs=1, type=str)

    def handle(self, *args, **options):
        try:
            College.objects.get(acronym=options['college_name'][0])
            print options['college_name'][0]
            college_report(options['college_name'][0])
        except:
            print "College not found"

