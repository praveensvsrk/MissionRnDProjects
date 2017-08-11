from rest_framework import routers, serializers, viewsets
from models import *


class CollegeSerializer(serializers.ModelSerializer):
    class Meta:
        model = College
        fields = ['id', 'name', 'acronym', 'location', 'contact']


class StudentSerializer(serializers.Serializer):
    id = serializers.IntegerField(label='ID', read_only=True)
    name = serializers.CharField(max_length=128)
    email = serializers.EmailField(max_length=254)
    dropped_out = serializers.BooleanField(required=False)
    db_folder = serializers.CharField(max_length=50)
    college = serializers.PrimaryKeyRelatedField(queryset=College.objects.all())
    # total_marks = serializers.RelatedField(source='mocktest1.total')

    def create(self, validated_data):
        return Student.objects.create(**validated_data)

    def update(self, instance, validated_data):
        instance.id = validated_data.get('id', instance.id)
        instance.name = validated_data.get('name', instance.name)
        instance.email = validated_data.get('email', instance.email)
        instance.dropped_out = validated_data.get('dropped_out', instance.dropped_out)
        instance.db_folder = validated_data.get('db_folder', instance.db_folder)
        instance.college = validated_data.get('college', instance.college)
        instance.save()
        return instance


class MarksSerializer(serializers.ModelSerializer):
    class Meta:
        model = MockTest1
        fields = ['problem1', 'problem2', 'problem3', 'problem4', 'total', 'student']

