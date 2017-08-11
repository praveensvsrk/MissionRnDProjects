# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models
from django.contrib.auth.models import User


class TodoList(models.Model):
    name = models.CharField(max_length=48)
    creation_date = models.DateField(auto_now_add=True)
    user = models.ForeignKey(User)

    def __unicode__(self):
        return self.name


class TodoItem(models.Model):
    description = models.CharField(max_length=254)
    completed = models.BooleanField(default=False)
    due_date = models.DateField(null=True, blank=True)
    list = models.ForeignKey(TodoList, related_name='items')

    def __unicode__(self):
        return self.description