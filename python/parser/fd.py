
import argparse
import os

from tarski.utils import resources


def generate_options_mechanism(domain, task, **kwargs):
    options = dict(
        domain=domain,
        task=task,
        generate_relaxed_task=False,
        use_partial_encoding=True,
        invariant_generation_max_candidates=100000,
        sas_file="output.sas",
        invariant_generation_max_time=300,
        add_implied_preconditions=False,
        filter_unreachable_facts=True,
        reorder_variables=True,
        filter_unimportant_vars=True,
        dump_task=False,
    )
    options.update(kwargs)
    return argparse.Namespace(**options)


def hijack_options_mechanism(domain, task, **kwargs):
    """ Monkey-patch the downward.translate.options module so that it doesn't attempt to read
     the command-line arguments as soon as it gets imported. Instead, we simply mock the module
     with an object that contains the expected attributes. """
    from downward import translate
    ns = generate_options_mechanism(domain, task, **kwargs)
    setattr(translate, 'options', ns)  # overwrite the translate.options module with a Namespace object


def parse_with_fd_translator(domain, instance, workdir):
    """ """
    hijack_options_mechanism(domain, instance,
                             sas_file=os.path.join(workdir, 'output.sas'))
    with resources.timing(f"Parsing problem with Fast Downward preprocessor", newline=True):
        from downward.translate import translate
        translate.main()  # This will parse everything and output the SAS file
