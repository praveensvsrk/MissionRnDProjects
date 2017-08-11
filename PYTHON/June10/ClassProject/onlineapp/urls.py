from django.conf.urls import url
from onlineapp import views, classviews
from rest_framework_jwt.views import obtain_jwt_token



urlpatterns = [
    # url(r'test', views.test),
    # url(r'^$', views.index),
    # url(r'test/', views.test_session),
    url(r'^login/$', classviews.LoginView.as_view(), name='login'),
    url(r'^logout/$', classviews.LogoutView.as_view(), name='logout'),
    url(r'^api-token-auth/', obtain_jwt_token),
    # url(r'except1/', views.except1),
    # url(r'except2/', views.except2),
    # url(r'headers/', views.test_headers),
    # url(r'cookie/', views.test_cookie),
    # url(r'colleges/$', classviews.CollegeListView.as_view(), name="CollegeListView"),
    # url(r'^students/', views.get_students),
    # url(r'colleges/(?P<pk>[0-9]+)/$', classviews.CollegeDetailView.as_view(), name='details'),
    # url(r'colleges/(?P<slug>[a-z]+)/$', classviews.CollegeDetailView.as_viewb(), name='details'),
    url(r'^colleges/create/$', classviews.CollegeCreate.as_view(), name='create_college'),
    # url(r'colleges/(?P<key>[0-9]+)/update/$', classviews.CollegeUpdate.as_view(), name='update_college'),
    # url(r'colleges/(?P<key>[0-9]+)/delete/$', classviews.CollegeDelete.as_view(), name='delete_college'),
    # url(r'colleges/([a-z]+)/$', views.college_details_acronym, name='details'),
    # url(r'colleges/([0-9]+)/$', views.college_details_id, name='details'),
    # url(r'colleges/([a-z]+)/$', views.college_details_acronym, name='details'),


    url(r'^colleges/$', views.CollegeList.as_view(), name='list_colleges'),
    url(r'^colleges/(?P<pk>[0-9]+)/$', views.CollegeDetail.as_view(), name='list_colleges_in_detail'),
    url(r'^colleges/(?P<pk>[0-9]+)/students/$', views.StudentListByCollege.as_view(), name='list_students_by_college'),
    url(r'^students/$', views.test_student_list, name='list_students'),
    url(r'^students/(?P<pk>[0-9]+)/$', views.StudentDetail.as_view(), name='list_students_in_detail'),
    url(r'^students/(?P<pk>[0-9]+)/marks/$', views.MarksList.as_view(), name='marks_detail'),

]


# http://localhost:8000/online
