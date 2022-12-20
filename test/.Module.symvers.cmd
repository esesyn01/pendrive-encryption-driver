cmd_/media/gui-shared/test/Module.symvers := sed 's/\.ko$$/\.o/' /media/gui-shared/test/modules.order | scripts/mod/modpost -m -a  -o /media/gui-shared/test/Module.symvers -e -i Module.symvers   -T -
