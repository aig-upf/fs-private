import subprocess


if __name__ == '__main__' :

    cmd = 'python run.py --cpus 4 --benchmark ../generated/novelty-experiments --secret password --planner "./nfs-planner.bin --max_novelty {0} {1}"'

    for i in range(1,5) :
        # run V+G over bound i
        subprocess.call( cmd, shell=True )
        # run V+G+A over bound i
        cmd_vga = cmd.format(i," --use_actions 1")
        subprocess.call( cmd_vga, shell=True )
        # run V over bound i
        cmd_v = cmd.format(i," --use_goal 0")
        subprocess.call( cmd_v, shell=True)
        # run V+A over bound i
        cmd_va = cmd.format(i, " --use_actions 1" )
        subprocess.call( cmd_va, shell=True)
        # run A over bound i
        cmd_a = cmd.format(i, " --use_state_vars 0 --use_goal 0 --use_actions 1")
        subprocess.call( cmd_a, shell=True)
        # run A+G over bound i
        cmd_ag = cmd.format(i, " --use_state_vars 0 --use_goal 1 --use_actions 1")
        subprocess.call( cmd_ag, shell=True)
