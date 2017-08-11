from debug_toolbar.panels import Panel
from django.template.loader import render_to_string
from django.utils.translation import ugettext_lazy as _
import psutil


def get_memory_stats():
    stats = {}
    stats['total'] = psutil.virtual_memory()[0]/(1024.0 * 1024)
    stats['available'] = psutil.virtual_memory()[1]/(1024.0 * 1024)
    stats['percent_used'] = psutil.virtual_memory()[2]
    stats['used'] = psutil.virtual_memory()[3]/(1024.0 * 1024)
    stats['free'] = psutil.virtual_memory()[4]/(1024.0 * 1024)
    return stats


def get_process_stats():
    stats = []
    for proc in psutil.process_iter():
        try:
            stats.append(proc.as_dict(attrs=['pid', 'name']))
        except psutil.NoSuchProcess:
            pass

    return stats


class SysInfo(Panel):
    name = 'SysInfo'
    has_content = True
    template = "panels/sysinfo.html"

    def nav_title(self):
        return _('SysInfo')

    def nav_subtitle(self):
        return "View system info"

    def title(self):
        return _('SysInfo')

    def process_response(self, request, response):
        self.record_stats({'memory': get_memory_stats(), 'processes': get_process_stats()})