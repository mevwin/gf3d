#ifndef __NOTIFICATIONS_H__
#define __NOTIFCATIONS_H__

#define NOTIF_TIME_MAX (3.0f)

// order must match with notifications.def
typedef enum NotifType_S {
    INVINCE_POWERUP,
    HAPPYTRIG_POWERUP,
    NO_SCRAP,
    DUPE_PERKS,
    NO_RUNS,
    LEVEL_SAVED,
    NOT_ENOUGH_ENEMIES,
    BOSS_ATTACK         // TODO
    
}NotifType;

void notif_window(NotifType notif_type);

#endif