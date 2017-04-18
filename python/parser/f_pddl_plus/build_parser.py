#!/usr/bin/python3
import sys, os, glob, shutil
import argparse

def parse_arguments() :
    parser = argparse.ArgumentParser(description='Build parser for one of the available grammars')
    parser.add_argument( '--grammar', required=True, help='Grammar to be built' )
    parser.add_argument( '--deploy', help ='Deploy generated files in given Python module')
    args = parser.parse_args()
    if not os.path.exists( args.grammar ) :
        print("Could not find grammar file '{0}'".format(args.grammar))
        sys.exit(1)
    return args

def main() :
    args = parse_arguments()

    antlr4_raw = 'java -Xmx500M -cp "/usr/local/lib/antlr-4.5-complete.jar:$CLASSPATH" org.antlr.v4.Tool -visitor'
    command = '{0} -Dlanguage=Python3 {1}'

    os.system( command.format(antlr4_raw, args.grammar) )

    # find prefix
    grammar_prefix = ''
    with open( args.grammar ) as instream:
        for line in instream :
            line = line.strip()
            if 'grammar' in line and line[-1] == ';':
                toks = line[0:-1].split(' ')
                grammar_prefix = toks[1]
                break

    print ("Generated files prefix: {0}".format(grammar_prefix))

    gen_files = glob.glob( '{0}*.py'.format(grammar_prefix) )

    if not os.path.exists( args.deploy ) :
        os.makedirs( args.deploy )
    os.system( 'touch {0}/__init__.py'.format( args.deploy ) )
    for name in gen_files :

        dst=shutil.move( name, os.path.join( args.deploy, name.replace(grammar_prefix,'').lower() ))
        if 'parser.py' in dst :
            with( open(dst) ) as instream :
                lines = instream.read()
            old_listener_module_1 = 'from {0}Listener'.format(grammar_prefix)
            old_visitor_module_1 = 'from {0}Visitor'.format(grammar_prefix)
            old_listener_module_2 = 'from .{0}Listener'.format(grammar_prefix)
            old_visitor_module_2 = 'from .{0}Visitor'.format(grammar_prefix)

            new_listener_module_1 = 'from listener'
            new_listener_module_2 = 'from .listener'
            new_visitor_module_1 = 'from visitor'
            new_visitor_module_2 = 'from .visitor'

            rules = [ ( old_listener_module_1, new_listener_module_1),
                      ( old_listener_module_2, new_listener_module_2),
                      (old_visitor_module_1, new_visitor_module_1),
                      (old_visitor_module_2, new_visitor_module_2)]
            print(rules)
            for old, new in rules :
                lines = lines.replace( old, new )

            with open( dst, 'w') as outstream  :
                outstream.write( lines )
            print( "Replaced old module names")
        print(dst)

if __name__ == '__main__' :
    main()
