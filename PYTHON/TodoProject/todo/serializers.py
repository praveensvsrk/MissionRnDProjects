from models import *
from rest_framework.serializers import ModelSerializer
import datetime


class TodoItemsSerializer(ModelSerializer):
    class Meta:
        model = TodoItem
        fields = ['id', 'description', 'due_date', 'completed']

    def create(self, validated_data):
        validated_data['list'] = TodoList.objects.get(pk=int(self.context['view'].kwargs['pk']))
        return super(TodoItemsSerializer, self).create(validated_data)

    def update(self, instance, validated_data):
        return super(TodoItemsSerializer, self).update(instance, validated_data)


class TodoListSerializer(ModelSerializer):
    items = TodoItemsSerializer(many=True)
    class Meta:
        model = TodoList
        fields = ['id', 'name', 'creation_date', 'items']

    def create(self, validated_data):
        validated_data['user'] = self.context['request'].user
        validated_data['creation_date'] = datetime.datetime.now()
        print validated_data
        items_data = validated_data.pop('items')
        list = TodoList.objects.create(**validated_data)
        for item_data in items_data:
            TodoItem.objects.create(list=list, **item_data)
        return list

