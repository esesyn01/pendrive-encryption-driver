#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/dmi.h>
#include <linux/fs.h>

MODULE_DESCRIPTION("Simple module");
MODULE_AUTHOR("Kernel Hacker");
MODULE_LICENSE("GPL");

static int get_uuid(void)
{
    const char *system_uuid;
    system_uuid = dmi_get_system_info(DMI_PRODUCT_UUID);
    printk(KERN_DEBUG "%s\n",system_uuid);
    return 0;
}

static int print_files()
{
    const char *dir_path = "/mount/minix/";
    struct inode *inode;
    struct dentry *entry;
    struct file *dir;

    // Otwórz katalog
    dir = filp_open(dir_path, O_RDONLY, 0);
    if (IS_ERR(dir)) {
        printk(KERN_ERR "Nie udało się otworzyć katalogu %s\n", dir_path);
        return PTR_ERR(dir);
    }

    // Pobierz wskaźnik do inode katalogu
    inode = dir->f_inode;

    // Pobierz wskaźnik do pierwszego pliku w katalogu
    entry = d_first(inode->i_dentry);

    // Przejdź przez wszystkie pliki w katalogu
    while (entry) {
        // Tutaj możesz wykonać operacje na pliku, na przykład wyświetlić jego nazwę
        printk(KERN_INFO "Znaleziono plik: %s\n", entry->d_name.name);

        // Pobierz wskaźnik do następnego pliku w katalogu
        entry = d_next(entry);
    }

    // Zamykamy katalog
    filp_close(dir, NULL);

    return 0;
}

static int my_hello_init(void)
{
    int err;
    err = print_files();
	printk(KERN_DEBUG "Hello!\n");
	return 0;
}

static void hello_exit(void)
{
	printk(KERN_DEBUG "Goodbye!\n");
}

module_init(my_hello_init);
module_exit(hello_exit);
