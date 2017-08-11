from models import *
from django.forms import ModelForm

class CollegeForm(ModelForm):
    class Meta:
        model = College
        fields = ['name', 'acronym', 'location', 'contact']