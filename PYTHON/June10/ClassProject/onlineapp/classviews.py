from django.views.generic.detail import DetailView
from django.views.generic.list import ListView
from models import *
from django.views.generic.edit import CreateView, UpdateView, DeleteView
from django.contrib.auth.mixins import PermissionRequiredMixin, LoginRequiredMixin
from forms import *
from django.urls import reverse_lazy
from django.db import connection, connections
from django.contrib.auth.views import LoginView, LogoutView


class CollegeListView(ListView):
    model = College
    context_object_name = 'colleges'
    lookup = 'location'

    # def get_queryset(self):
    #     try:
    #         return self.model.objects.filter(**{self.lookup: self.request.GET.get(self.lookup, None)})
    #     except:
    #         return self.model.objects.all()


class CollegeDetailView(DetailView):
    model = College
    template_name = "onlineapp/student_list.html"
    slug_field = 'acronym'

    def get_context_data(self, **kwargs):
        context = super(CollegeDetailView, self).get_context_data(**kwargs)
        college = context['college']
        print context
        context['students'] = Student.objects.values('name', 'mocktest1__total')\
            .filter(college=college).order_by('-mocktest1__total')
        return context


class CollegeCreate(LoginRequiredMixin, PermissionRequiredMixin, CreateView):
    login_url = '/online/login/'
    login_redirect_url = '/online/colleges/'
    permission_required = 'onlineapp.add_college'
    raise_exception = False
    permission_denied_message = "dasdadjhshadh"
    model = College
    fields = ['name', 'acronym', 'location', 'contact']
    success_url = reverse_lazy('CollegeListView')


class CollegeUpdate(UpdateView):
    model = College
    template_name = 'onlineapp/college_form.html'
    fields = ['name', 'acronym', 'location', 'contact']
    success_url = reverse_lazy('CollegeListView')

    def get_object(self, queryset=None):
        return self.model.objects.get(pk=int(self.kwargs['key']))


class CollegeDelete(DeleteView):
    model = College
    success_url = reverse_lazy('CollegeListView')

    def get_object(self, queryset=None):
        return self.model.objects.get(pk=int(self.kwargs['key']))

