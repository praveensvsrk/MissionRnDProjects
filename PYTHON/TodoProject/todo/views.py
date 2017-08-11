# -*- coding: utf-8 -*-
from __future__ import unicode_literals
from django.http import HttpResponse, HttpResponseRedirect
from django.shortcuts import render
from django.views import View
from django.views.generic import CreateView, DeleteView, UpdateView, ListView, DetailView, TemplateView
from django.contrib.auth.mixins import LoginRequiredMixin, PermissionRequiredMixin
from models import *
from forms import *
from django.contrib.auth.views import LoginView, LogoutView
from django.urls import reverse_lazy
from rest_framework.generics import RetrieveUpdateDestroyAPIView, ListCreateAPIView, CreateAPIView
from serializers import *
from rest_framework_jwt.authentication import JSONWebTokenAuthentication
from rest_framework.permissions import BasePermission, DjangoObjectPermissions, IsAuthenticated
from django.contrib.auth.models import AnonymousUser


class IsOwner(BasePermission):
    def has_object_permission(self, request, view, obj):
        if (obj.__class__.__name__ == 'TodoItem' and obj.list.user == request.user) or obj.user == request.user:
            return True
        else:
            return False


class TodoListRUDView(RetrieveUpdateDestroyAPIView):
    serializer_class = TodoListSerializer
    queryset = TodoList.objects.all()
    authentication_classes = [JSONWebTokenAuthentication]
    permission_classes = [IsAuthenticated, IsOwner]


class TodoListCRView(ListCreateAPIView):
    serializer_class = TodoListSerializer
    queryset = TodoList.objects.all()
    authentication_classes = [JSONWebTokenAuthentication]
    permission_classes = [IsAuthenticated, IsOwner]

    def get_queryset(self):
        print self.request.user.get_username()
        if self.request.user and self.request.user.get_username() != 'AnonymousUser':
            return TodoList.objects.filter(user=self.request.user)
        else:
            return None


class TodoItemCView(CreateAPIView):
    serializer_class = TodoItemsSerializer
    queryset = TodoItem.objects.all()
    authentication_classes = [JSONWebTokenAuthentication]
    permission_classes = [IsAuthenticated, IsOwner]


class TodoItemRUDView(RetrieveUpdateDestroyAPIView):
    serializer_class = TodoItemsSerializer
    queryset = TodoItem.objects.all()
    authentication_classes = [JSONWebTokenAuthentication]
    permission_classes = [IsAuthenticated, IsOwner]
