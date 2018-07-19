#include <string.h>
#include <stdlib.h>
#include "live_update.h"
#include "logcat.h"

#define LIVE_UPDATE	"live_update"

static void __live_update()
{
	system("wget 192.168.0.117/bin/module-manager");
	system("rm /etc/3team/module-manager")
	system("cp /etc/3team/module-manager /usr/bin");
	system("rebbot");
	
	return;
}

void* live_update_check_cb(struct info_t *info)
{
	JSON_Value *rootValue;
	JSON_Object *rootObject;

	rootValue = json_parse_string(info->receive_msg);
	rootObject = json_value_get_object(rootValue);

	if (strcmp(json_object_get_string(rootObject, "cmd"), LIVE_UPDATE) != 0) {
		goto RET;
	}

	log_d("[key : %s] [data : %s]\n", "cmd", json_object_get_string(rootObject, "cmd"));

	__live_update();

RET:
	json_value_free(rootValue);

	return NULL;
}

