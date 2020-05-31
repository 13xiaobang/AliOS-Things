#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <misc/printk.h>
#include <misc/byteorder.h>
#include <zephyr.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

static struct bt_gatt_ccc_cfg privc_ccc_cfg[BT_GATT_CCC_MAX] = {};
static u8_t simulate_priv = 0;
static u16_t priv_step = 0;

static void privc_ccc_cfg_changed(const struct bt_gatt_attr *attr,
				 u16_t value)
{
	printf("privc_ccc_cfg_changed, value=%d\n", value);
	simulate_priv = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

static ssize_t read_blsc(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 void *buf, u16_t len, u16_t offset)
{
	printf("host read=%d\n", priv_step);
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &priv_step,
				 sizeof(priv_step));
}

static ssize_t write_blsc(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			const void *buf, u16_t len, u16_t offset, u8_t flags)
{
	int i = 0;
	printf("host write: \n");
	for(; i<len; i++)
	{
		printf("%x ", ((char*)buf)[i]);
	}
	printf("\n");
        //every time host write, reset step.
        priv_step = 0;
        
}

/* Step Service Declaration */
static struct bt_gatt_attr attrs[] = {
	BT_GATT_PRIMARY_SERVICE(BT_UUID_PRIV_DATA),
	BT_GATT_CHARACTERISTIC(BT_UUID_PRIV_CHW_DATA, BT_GATT_CHRC_WRITE),
	BT_GATT_DESCRIPTOR(BT_UUID_PRIV_CHW_DATA, BT_GATT_PERM_WRITE, NULL,
			   write_blsc, NULL),
	BT_GATT_CHARACTERISTIC(BT_UUID_PRIV_CHR_DATA, BT_GATT_CHRC_READ),
	BT_GATT_DESCRIPTOR(BT_UUID_PRIV_CHR_DATA, BT_GATT_PERM_READ,
			   read_blsc, NULL, NULL),
	BT_GATT_CHARACTERISTIC(BT_UUID_PRIV_CHN_DATA, BT_GATT_CHRC_NOTIFY),
	/* TODO: Add write permission and callback */
	BT_GATT_DESCRIPTOR(BT_UUID_PRIV_CHN_DATA, BT_GATT_PERM_READ, NULL,
			   NULL, NULL),
	BT_GATT_CCC(privc_ccc_cfg, privc_ccc_cfg_changed),
};

static struct bt_gatt_service priv_svc = BT_GATT_SERVICE(attrs);

void priv_init(u8_t blsc)
{
	priv_step = blsc;
	bt_gatt_service_register(&priv_svc);
}

void priv_notify(void)
{
	int err = 0;
	/* step increase 1 every second*/
        priv_step++;

        //if notify close ,return.
	if (!simulate_priv) {
		return;
	}

        if(priv_step%5  == 0) {
	    err = bt_gatt_notify(NULL, &attrs[6], &priv_step, sizeof(priv_step));
	    printf("simulate_priv notify = %d, err = %d\n", priv_step, err);
        }
}
