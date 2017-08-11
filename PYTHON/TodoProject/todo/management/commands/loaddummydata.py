from django.core.management import BaseCommand
from todo.models import *
from openpyxl import load_workbook
import os

def get_sheet_data(sheet):
    data = []
    row_count = len(sheet.rows)

    for row in range(1, row_count):
        row_data = [cell.value if not cell.value is None else None for cell in sheet.rows[row]]
        for i in range(len(row_data)):

            if type(row_data[i]).__name__ == 'datetime':
                row_data[i] = str(row_data[i].date())
            elif str(row_data[i]) == 'True':
                row_data[i] = True
            elif str(row_data[i]) == 'False':
                row_data[i] = False
            else:
                if row_data[i] == 'None':
                    row_data[i] = None

        data.append(row_data)

    return data


class Command(BaseCommand):
    help = "Randomly loads data into the todo lists"

    def handle(self, *args, **options):
        print os.getcwd()
        wb = load_workbook('test.xlsx')

        lists = get_sheet_data(wb.get_sheet_by_name('Lists'))
        list_items = get_sheet_data(wb.get_sheet_by_name('Items'))

        list_objects = []
        list_item_objects = []

        for list in lists:
            list_objects.append(TodoList(
                name=list[0],
                creation_date=list[1]
            ))
        TodoList.objects.bulk_create(list_objects)

        for item in list_items:
            list_item_objects.append(TodoItem(
                description=item[0],
                completed=item[1],
                due_date=item[2],
                list=TodoList.objects.get(name="list" + str(item[3]))
            ))
        TodoItem.objects.bulk_create(list_item_objects)