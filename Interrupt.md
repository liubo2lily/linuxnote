# 0 前言

中断最重要的就是中断号，irq。

# 1 注册流程

```c
gpio_keys_100ask[i].irq  = gpio_to_irq(gpio_keys_100ask[i].gpio);
err = request_irq(gpio_keys_100ask[i].irq, gpio_key_isr, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "100ask_gpio_key", &gpio_keys_100ask[i]);
static irqreturn_t gpio_key_isr(int irq, void *dev_id)
{
	struct gpio_key *gpio_key = dev_id;
	int val;
	val = gpiod_get_value(gpio_key->gpiod);

	printk("key %d %d\n", gpio_key->gpio, val);
	
	return IRQ_HANDLED;
}
free_irq(gpio_keys_100ask[i].irq, &gpio_keys_100ask[i]);
```

