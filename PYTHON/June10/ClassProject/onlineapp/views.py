# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import connection, connections
from django.http import HttpResponse, Http404, JsonResponse
from django.shortcuts import render
from django.utils.decorators import method_decorator

from models import *
from django.template import loader
from rest_framework.renderers import JSONRenderer
from rest_framework.parsers import JSONParser
from django.views.decorators.csrf import csrf_exempt
from serializers import *
from rest_framework import mixins, generics
from rest_framework.permissions import IsAuthenticated
from rest_framework_jwt.authentication import JSONWebTokenAuthentication
from rest_framework_jwt.views import obtain_jwt_token

# Create your views here.

count = 0


def index(request):
    return HttpResponse("Hello")


def test_session(request):
    request.session.setdefault('count', 0)
    counter = request.session['count']
    counter += 1
    request.session['count'] = counter
    response = HttpResponse(str(counter))
    response.set_cookie("onlinecookie_" + str(counter), "Have this cookie!", 60)
    return response


def except1(request):
    raise ValueError


def except2(request):
    raise Http404("except2 here")


def test_cookie(request):
    return HttpResponse(request.COOKIES.__repr__().replace(',', ',<br>'))


def test_headers(request):
    return HttpResponse("Hello test")


def get_colleges(request):
    colleges = College.objects.all()
    print connections['default'].queries

    return render(request, 'onlineapp/college_list.html', {'colleges': colleges})


def get_students(request):
    students = Student.objects.all()
    template = loader.get_template('onlineapp/students.html')
    return HttpResponse(template.render({'students': students, 'header': 'All students'}, request))


def college_details_id(request, college_id):
    college_id = int(college_id)
    students = Student.objects.filter(college__id=int(college_id)).values('name', 'mocktest1__total').order_by(
        '-mocktest1__total')
    return render(request, 'onlineapp/students.html',
                  {'students': students, 'college': College.objects.get(pk=college_id).acronym})


def college_details_acronym(request, college_acronym):
    students = Student.objects.filter(college__acronym=college_acronym).values('name', 'mocktest1__total').order_by(
        '-mocktest1__total')
    return render(request, 'onlineapp/students.html',
                  {'students': students, 'college': college_acronym})


@csrf_exempt
def test_student_list_by_college(request, pk):
    try:
        college = College.objects.get(pk=pk)
        students = Student.objects.filter(college=college)
    except College.DoesNotExist:
        return HttpResponse(status=404)
    except Student.DoesNotExist:
        return HttpResponse(status=404)

    if request.method == 'GET':
        serializer = StudentSerializer(students, many=True)
        return JsonResponse(serializer.data, safe=False)

    if request.method == 'POST':
        data = JSONParser().parse(request)
        data['college'] = pk
        serializer = StudentSerializer(data=data)
        if serializer.is_valid():
            serializer.save()
            return JsonResponse(serializer.data, status=201)
        return JsonResponse(serializer.errors, status=400)

@csrf_exempt
def test_student_list(request):
    if request.method == 'GET':
        students = Student.objects.all()
        serializer = StudentSerializer(students, many=True)
        return JsonResponse(serializer.data, safe=False)

    if request.method == 'POST':
        data = JSONParser().parse(request)
        serializer = StudentSerializer(data=data)
        if serializer.is_valid():
            serializer.save()
            return JsonResponse(serializer.data, status=201)
        return JsonResponse(serializer.errors, status=400)



@csrf_exempt
def test_student_detail(request, pk):
    print pk
    try:
        student = Student.objects.get(pk=pk)
    except Student.DoesNotExist:
        return HttpResponse(status=404)

    if request.method == 'GET':
        serializer = StudentSerializer(student)
        return JsonResponse(serializer.data)

    if request.method == 'PUT':
        data = JSONParser().parse(request)
        serializer = StudentSerializer(student, data=data)
        if serializer.is_valid():
            serializer.save()
            return JsonResponse(serializer.data)
        return JsonResponse(serializer.errors, status=400)

    if request.method == 'DELETE':
        student.delete()
        return HttpResponse(status=204)


@csrf_exempt
def test_marks_detail(request, pk):
    try:
        marks = MockTest1.objects.get(student__id=pk)
    except MockTest1.DoesNotExist:
        return HttpResponse(status=404)

    if request.method == 'GET':
        serializer = MarksSerializer(marks)
        return JsonResponse(serializer.data)

    if request.method == 'PUT':
        data = JSONParser().parse(request)
        serializer = MarksSerializer(marks, data=data)
        if serializer.is_valid():
            serializer.save()
            return JsonResponse(serializer.data)
        return JsonResponse(serializer.errors, status=400)

    if request.method == 'POST':
        data = JSONParser().parse(request)
        serializer = MarksSerializer(data=data)
        if serializer.is_valid():
            serializer.save()
            return JsonResponse(serializer.data, status=201)
        return JsonResponse(serializer.errors, status=400)

    if request.method == 'DELETE':
        marks.delete()
        return HttpResponse(status=204)


class CollegeList(generics.ListCreateAPIView):
    serializer_class = CollegeSerializer
    queryset = College.objects.all()
    authentication_classes = []
    permission_classes = []


class CollegeDetail(generics.RetrieveUpdateDestroyAPIView):
    serializer_class = CollegeSerializer
    queryset = College.objects.all()
    authentication_classes = []
    permission_classes = []

    def put(self, request, *args, **kwargs):
        kwargs['partial'] = True
        return super(CollegeDetail, self).put(request, *args, **kwargs)


class StudentListByCollege(generics.ListCreateAPIView):
    serializer_class = StudentSerializer
    queryset = Student.objects.all()
    authentication_classes = []
    permission_classes = []

    def get(self, request, *args, **kwargs):
        self.queryset = Student.objects.filter(college_id=self.kwargs['pk'])
        return super(StudentListByCollege, self).get(request, *args, **kwargs)

    def post(self, request, *args, **kwargs):
        request.data['college'] = int(kwargs['pk'])
        return super(StudentListByCollege, self).post(request, *args, **kwargs)


class StudentDetail(generics.RetrieveUpdateDestroyAPIView):
    serializer_class = StudentSerializer
    queryset = Student.objects.all()
    authentication_classes = [JSONWebTokenAuthentication]
    permission_classes = [IsAuthenticated]

    def put(self, request, *args, **kwargs):
        kwargs['partial'] = True
        return super(StudentDetail, self).put(request, *args, **kwargs)


class MarksList(generics.ListCreateAPIView):
    serializer_class = MarksSerializer
    queryset = MockTest1.objects.all()
    authentication_classes = []
    permission_classes = []