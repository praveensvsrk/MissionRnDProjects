from django.conf.urls import url
from . import views
from rest_framework_jwt.views import obtain_jwt_token
from django.views.generic import TemplateView


urlpatterns = [
    url(r'^api-token-auth/', obtain_jwt_token),
    url(r'^api/lists/$', views.TodoListCRView.as_view(), name="ListView"),
    url(r'^login/$', views.LoginView.as_view(), name="LoginView"),
    url(r'^logout/$', views.LogoutView.as_view(), name='LogoutView'),
    url(r'^api/lists/(?P<pk>[0-9]+)/$', views.TodoListRUDView.as_view(), name="ListItemsView"),
    url(r'^api/lists/(?P<pk>[0-9]+)/items/$', views.TodoItemCView.as_view(), name="ItemView"),
    url(r'^api/items/(?P<pk>[0-9]+)/$', views.TodoItemRUDView.as_view(), name="ItemDetailView"),
    url(r'^lists/$', TemplateView.as_view(template_name="index.html")),
    url(r'^lists/list_template.html/$', TemplateView.as_view(template_name="list_template.html")),
    url(r'^lists/items_template.html/$', TemplateView.as_view(template_name="items_template.html")),
    url(r'^lists/item_form_template.html/$', TemplateView.as_view(template_name="item_form_template.html")),
    url(r'^lists/list_form_template.html/$', TemplateView.as_view(template_name="list_form_template.html")),
    url(r'^lists/legend_template.html/$', TemplateView.as_view(template_name="legend_template.html")),
    # url(r'^lists/items_template.html/$', TemplateView.as_view(template_name="items_template.html")),
    # url(r'^lists/(?P<pk>[0-9]+)/view/$', views.TodoListDetailView.as_view(), name="items"),
    # url(r'^lists/(?P<pk>[0-9]+)/update/$', views.TodoListUpdateView.as_view(), name="ListUpdate"),
    # url(r'^lists/(?P<pk>[0-9]+)/delete/$', views.TodoListDeleteView.as_view(), name="ListDelete"),

]
