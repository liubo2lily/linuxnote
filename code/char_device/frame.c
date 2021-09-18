/* char device driver frame base on platform bus */

static int major = 0;
static struct class *xxx_class;

static int xxx_probe(struct platform_device *pdev)
{
    struct resource *mem;
    xxx = devm_kzalloc(&pdev->dev, sizeof(struct s_xxx), GFP_KERNEL);
    if (!xxx)
		return -ENOMEM;
    mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    paddr = (unsigned long)(mem->start);
    regs = devm_ioremap_resource(&pdev->dev, mem);
    if (IS_ERR(regs)) {
		dev_err(&pdev->dev, "pmon region map failed\n");
		return PTR_ERR(regs);
	}
    major = register_chrdev(0, "char_name", &xxx_fops);
    xxx_class = class_create(THIS_MODULE, "char_name");
	if (IS_ERR(xxx_class)) {
		unregister_chrdev(major, "char_name");
		return PTR_ERR(xxx_class);
	}
    device_create(xxx_class, NULL, MKDEV(major, 0), NULL, "char_name");
    return 0;
    platform_set_drvdata(pdev, priv_data);
}

static int xxx_remove(struct platform_device *pdev)
{
    priv_data = (struct priv_data*)platform_get_drvdata(pdev);  
	device_destroy(xxx_class, MKDEV(major, 0));
	class_destroy(xxx_class);
	unregister_chrdev(major, "char_name");
	return 0;
}

static const struct of_device_id xxx_table[] = {
    { .compatible = "chipx,name" },
    { },
};

static struct platform_driver xxx_driver = {
    .probe      = xxx_probe,
    .remove     = xxx_remove,
    .driver     = {
        .name   = "char_name",
        .of_match_table = xxx_table,
    },
};

static int __init xxx_driver_init(void)
{
    int err;   	
    err = platform_driver_register(&xxx_driver); 	
	return err;
}

static void __exit xxx_driver_exit(void)
{
    platform_driver_unregister(&xxx_driver);
}

module_init(xxx_driver_init);
module_exit(xxx_driver_exit);
MODULE_LICENSE("GPL");