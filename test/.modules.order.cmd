cmd_/media/gui-shared/test/modules.order := {   echo /media/gui-shared/test/encryption.ko; :; } | awk '!x[$$0]++' - > /media/gui-shared/test/modules.order
