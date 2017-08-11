
# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models
from django.db.models import Count


#createdb dropdb populatedb(excel, html) cleardata
#TODO: Why are querysets executing when printing? they are not suppossed to do so
#TODO: Enable logging at sql console level

# Create your models here.
class College(models.Model):
    name = models.CharField(max_length=128)
    location = models.CharField(max_length=64)
    acronym = models.CharField(max_length=8)
    contact = models.EmailField()

    def __unicode__(self):
        return self.acronym


class Student(models.Model):
    name = models.CharField(max_length=128)
    dob = models.DateField(null=True, blank=True)
    email = models.EmailField()
    db_folder = models.CharField(max_length=50)
    college = models.ForeignKey(College)
    dropped_out = models.BooleanField(default=False)

    def __unicode__(self):
        return self.name


class MockTest1(models.Model):
    problem1 = models.IntegerField()
    problem2 = models.IntegerField()
    problem3 = models.IntegerField()
    problem4 = models.IntegerField()
    total = models.IntegerField()
    student = models.OneToOneField(Student)


def queries():
    # all colleges
    print "1"
    c = College.objects.all()
    s = Student.objects.all()

    # count of colleges
    print "2"
    print c.count()

    # get acronym and contact for each college
    print "3"
    print c.values('acronym', 'contact')

    # get college count in vizag
    print "4"
    print c.filter(location='Vizag').count()

    # sort by acronym
    print "5"
    print c.order_by('acronym')

    # sort by location desc
    print "6"
    print c.order_by('-location')

    # sort by location desc top 5
    print "7"
    print c.order_by('-location')[:5]

    # how many colleges in each location
    print "8"
    print c.values('location').annotate(count=Count('location'))

    # colleges in each location sort by location
    print "9"
    print c.values('acronym', 'location').order_by('location')

    # colleges in each location sort by num colleges desc
    print "10"
    print c.values('location').annotate(count=Count('location')).order_by('-count')

    # get acronym and contact for each college sorted by location
    print "11"
    print c.values('acronym', 'contact').order_by('location')

    # get colleges who have dropped out students
    print "12"
    print c.filter(student__dropped_out=True).distinct()

    # OR s.filter(dropped_out = True).values('college__acronym', 'college__name')

    # get colleges who do not have dropped out students
    print "13"
    print c.exclude(student__dropped_out=True)

    # OR s.filter(dropped_out = False).values('college__acronym', 'college__name')

    # get count of all students
    print "14"
    print s.count()

    # get students who have rohit in their name
    print "15"
    print s.filter(name__icontains="rohit")

    # get all students from bvritn
    print "16"
    print s.filter(college__acronym='bvritn')

    # get college acronym and count of students in that college
    print "17"
    print s.values('college__name').annotate(count=Count('college'))

    # get college acronym and count of students in that college in sorted desc count
    print "18"
    print s.values('college__name').annotate(count=Count('college')).order_by('-count')

    # get college acronym and count of students in that college in sorted desc count - 2
    print "19"
    print "really?"

    # annotate each student with his college acronym order by college acronym
    print "20"
    print s.values('name', 'college__acronym').order_by('college__acronym')

    # annnotate student count in each college and sort by count desc
    print "21"
    print s.values('college__acronym').annotate(count=Count('name')).order_by('-count')

    # annnotate student count in each college and get all colleges with more than 10 students sort by count desc
    print "22"
    print c.annotate(num_students = Count('student')).filter(num_students__gt=10)
    print s.values('college__acronym').annotate(count=Count('name')).order_by('-count').exclude(count__lte=10)

    # get location and student count sorted by desc
    print "23"
    print s.values('college__location').annotate(count=Count('name')).order_by('-count')

    # get location with max student count


#Todo list - name, creation date
#Todo Item - description, completed, due_by_date(optional)
#management command loaddummydata - populate 5 lists with 5 items in each