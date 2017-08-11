from django import forms
from models import *


class TodoItemForm(forms.ModelForm):

    class Meta:
        model = TodoItem
        fields = ['description', 'due_date', 'list']
