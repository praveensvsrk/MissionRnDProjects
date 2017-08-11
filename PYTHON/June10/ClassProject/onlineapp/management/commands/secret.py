from django.core.management.base import BaseCommand
import click


class Command(BaseCommand):
    help = "prints it"

    def handle(self, *args, **options):
        click.secho('I\'m a cool Django developer!', fg='green')