# -*- coding: utf-8 -*-

from rest_framework.test import APITestCase
from rest_framework import status
from django.urls import reverse_lazy, reverse
import json
from django import http


#TODO: POST colleges/id/students
#TODO: PUT students/id
#TODO: DELETE students/id
#TODO: students/id/marks


class CollegeTests(APITestCase):
    data = {
            "id": 1,
            "name": "CVR College of Engineering",
            "acronym": "cvr",
            "location": "Hyderabad",
            "contact": "contact@cvr.edu"
            }

    id = None

    def test_college(self):
        url = '/online/colleges/'
        response = self.client.post(url, self.data, format='json')
        self.id = json.loads(response.content)['id']
        self.assertEqual(response.status_code, status.HTTP_201_CREATED)

        url = '/online/colleges/' + str(self.id) + '/'
        response = self.client.get(url)
        self.assertEqual(response.status_code, status.HTTP_200_OK)

        response = self.client.put(url, {
            "id": 1,
            "name": "CVR College of Engineering",
            "acronym": "cvr2",
            "location": "Hyderabad",
            "contact": "contact@cvr.edu"
        }, format='json')
        response = self.client.get(url)
        self.assertEqual(json.loads(response.content), {
            "id": 1,
            "name": "CVR College of Engineering",
            "acronym": "cvr2",
            "location": "Hyderabad",
            "contact": "contact@cvr.edu"
        })

        response = self.client.delete(url)
        self.assertEqual(response.status_code, status.HTTP_204_NO_CONTENT)
        response = self.client.get(url)
        self.assertEqual(response.status_code, status.HTTP_404_NOT_FOUND)
