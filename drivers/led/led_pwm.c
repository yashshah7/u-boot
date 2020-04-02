// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <led.h>
#include <dm/lists.h>
#include <pwm.h>

struct led_pwm_priv {
	struct udevice *pwm;
	uint channel;
	uint period_ns;
	bool enabled;
};

static int pwm_led_set_state(struct udevice *dev, enum led_state_t state)
{
	struct led_pwm_priv *priv = dev_get_priv(dev);
	uint duty_cycle;
	int ret;

	switch (state) {
	case LEDST_OFF:
		ret = pwm_set_enable(priv->pwm, priv->channel, false);
		if (ret)
			return log_ret(ret);
		priv->enabled = 0;
		break;
	case LEDST_ON:
		duty_cycle = priv->period_ns / 2;
		ret = pwm_set_config(priv->pwm, priv->channel, priv->period_ns, duty_cycle);
		if (ret)
			return log_ret(ret);
	
		ret = pwm_set_enable(priv->pwm, priv->channel, true);
		if (ret)
			return log_ret(ret);
		priv->enabled = 1;
		break;
	case LEDST_TOGGLE:
		break;
	default:
		return -ENOSYS;
	}

	return 0;
}

static enum led_state_t pwm_led_get_state(struct udevice *dev)
{
	struct led_pwm_priv *priv = dev_get_priv(dev);
	int ret;

	ret = priv->enabled;

	return ret ? LEDST_ON : LEDST_OFF;
}

static int led_pwm_probe(struct udevice *dev)
{
	struct led_uc_plat *uc_plat = dev_get_uclass_platdata(dev);
	struct led_pwm_priv *priv = dev_get_priv(dev);
	struct ofnode_phandle_args args;
	int ret;
	uint duty;

	/* Ignore the top-level LED node */
	if (!uc_plat->label)
		return 0;

	ret = dev_read_phandle_with_args(dev, "pwms", "#pwm-cells", 0, 0, &args);
	if (ret) {
		log_err("Cannot get pwm handle: ret=%d\n", ret);
		return log_ret(ret);
	}

	ret = uclass_get_device_by_ofnode(UCLASS_PWM, args.node, &priv->pwm);
	if (ret) {
		log_err("Cannot get pwm: ret=%d\n", ret);
		return log_ret(ret);
	}
	priv->channel = args.args[0];
	priv->period_ns = args.args[1];

	duty = priv->period_ns;
	pwm_set_config(priv->pwm, priv->channel, priv->period_ns, duty);
	
	return 0;
}

static int led_pwm_remove(struct udevice *dev)
{
	/*
	 * The GPIO driver may have already been removed. We will need to
	 * address this more generally.
	 */
//	struct led_pwm_priv *priv = dev_get_priv(dev);

	return 0;
}

static int led_pwm_bind(struct udevice *parent)
{
	struct udevice *dev;
	ofnode node;
	int ret;

	dev_for_each_subnode(node, parent) {
		struct led_uc_plat *uc_plat;
		const char *label;

		label = ofnode_read_string(node, "label");
		if (!label) {
			debug("%s: node %s has no label\n", __func__,
			      ofnode_get_name(node));
			return -EINVAL;
		}
		ret = device_bind_driver_to_node(parent, "pwm_led",
						 ofnode_get_name(node),
						 node, &dev);
		if (ret)
			return ret;
		uc_plat = dev_get_uclass_platdata(dev);
		uc_plat->label = label;
	}

	return 0;
}

static const struct led_ops pwm_led_ops = {
	.set_state	= pwm_led_set_state,
	.get_state	= pwm_led_get_state,
};

static const struct udevice_id led_pwm_ids[] = {
	{ .compatible = "pwm-leds" },
	{ }
};

U_BOOT_DRIVER(led_pwm) = {
	.name	= "pwm_led",
	.id	= UCLASS_LED,
	.of_match = led_pwm_ids,
	.ops	= &pwm_led_ops,
	.priv_auto_alloc_size = sizeof(struct led_pwm_priv),
	.bind	= led_pwm_bind,
	.probe	= led_pwm_probe,
	.remove	= led_pwm_remove,
};
