
import json
import os
import subprocess
from statistics import mean


import pandas as pd

pd.set_option('display.max_colwidth', -1)
pd.set_option('display.max_columns', None)  

here = os.path.abspath(os.path.dirname(__file__))


def parse_json(filename):
    with open(filename, 'r') as f:
        data = json.load(f)
    return data


def generate_latex_report(content, filename):
    with open(os.path.join(here, 'tpl.tex'), 'r') as f:
        tpl = f.read()
    output = os.path.join(here, filename)
    with open(output, 'w') as f:
        print(tpl.replace('#Content#', content), file=f)
    subprocess.run(["ls", "-l"])
    subprocess.run(f"pdflatex {filename}".split(),  check=True)


def generate_latex_section(title, content):
    return f'\\section{{{title}}}\n\n{content}'


def generate_latex_pagebreak():
    return f'\\newpage'


def layout_results(results, means, aggrs):
    domains = sorted(set(k[0] for k in results.keys()))
    per_domain = []
    per_instance = []

    for d in domains:
        instances = [v for k, v in results.items() if k[0] == d]
        summary = {'problem': d}
        for a in means:
            summary[a] = mean(v[a] for v in instances)
        for a in aggrs:
            summary[a] = sum(v[a] for v in instances)
        per_domain.append(summary)

        per_instance.append((d, pd.DataFrame(instances)))

    total_summary = {'problem': 'Summary'}
    for a in means:
        total_summary[a] = mean(v[a] for v in per_domain)
    for a in aggrs:
        total_summary[a] = sum(v[a] for v in per_domain)

    return pd.DataFrame(per_domain + [total_summary]), per_instance
