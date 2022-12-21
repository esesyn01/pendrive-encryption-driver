#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/dmi.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <crypto/aes.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/crypto.h>
#include <crypto/skcipher.h>
#include <linux/scatterlist.h>

#define FIRST_DATA_ZONE 696
#define ZONESIZE 1024


MODULE_DESCRIPTION("This kernel module encrypts or decrypts received disk image, with MINIX filesystem.");
MODULE_AUTHOR("Wiktor Kawka 184417");
MODULE_LICENSE("GPL");

// getting parameters
static int mode = -1;
static char *disk_name = "dddddddddddddddddddddddddddddddddddddddddddefault";
module_param(mode, int, 0);
module_param(disk_name, charp, 0);

// getting the UUID - it will be the key for encryption
static char * get_uuid(void)
{
    const char *system_uuid;
    system_uuid = dmi_get_system_info(DMI_PRODUCT_UUID);
    char *result = kmalloc(40, GFP_KERNEL);
    if (!result)
    {
        return NULL;
    }

    int i=0,j=0;

    // eliminate dashes from string
    for (i=0, j=0; i<strlen(system_uuid); i++)
    {
        if (system_uuid[i] != '-')
        {
            result[j] = system_uuid[i];
            j++;
        }
    }

    result[j] = '\0';
    return result;
}

// function responsible for encrypting or decrypting one datazone.
static char * encrypt_or_decrypt_zone(char *in_buffer, struct crypto_skcipher *tfm, int len)
{
    char *out_buffer = kmalloc(len, GFP_KERNEL);
    
    struct scatterlist sg_in; // allocate scatterlist structs
    sg_init_one(&sg_in, in_buffer, len);

    struct scatterlist sg_out;
    sg_init_one(&sg_out, out_buffer, len);

    u8 iv[32]; // set the IV
    memset(iv, 0, 32);

    struct skcipher_request *req;
    req = skcipher_request_alloc(tfm, GFP_KERNEL); // allocate the request
    if (!req) {
        crypto_free_skcipher(tfm);
        printk(KERN_ERR "Cannot allocate request\n");
        return NULL;
    }

    skcipher_request_set_tfm(req, tfm); // set the cipher
    skcipher_request_set_callback(req, 0, NULL, NULL); // omit the callback
    skcipher_request_set_crypt(req, &sg_in, &sg_out, len, iv); // set fileds in request

    int ret;
    if (mode == 0)
    {
        ret = crypto_skcipher_encrypt(req); // encrypt chunk
    }
    else
    {
        ret = crypto_skcipher_decrypt(req); // decrypt chunk
    }
    if (ret) {
        printk(KERN_ERR "Cannot perform encryption/decryption\n");
        return NULL;
    }

    sg_copy_to_buffer(&sg_out, sg_nents(&sg_out), out_buffer, len); // copy result to buffer
    skcipher_request_free(req);

    return out_buffer;
}

// main function for encrypting/decrypting image
static int encrypt_or_decrypt(char *s)
{
    struct file *filp;
    filp = filp_open(s, O_RDWR, S_IWUSR | S_IRUSR); //open the file
    int i = 0;
    size_t bytes_write = 0;
    size_t count = ZONESIZE;
    loff_t pos_r = 0, pos_w = 0;
    ssize_t ret;

    char *uuid = get_uuid(); //get key and transform to needed u8 type
    u8* u8_uuid = (u8 *)uuid;

    struct crypto_skcipher *tfm; // allocate crypto_skcipher struct
    tfm = crypto_alloc_skcipher("cbc(aes)", 0, 0);
    if (IS_ERR(tfm)) {
        return PTR_ERR(tfm);
    }

    if (!filp)
    {
        return -ENOENT;
    }

    char *buf;
    buf = kmalloc(count, GFP_KERNEL);

    if(!buf)
    {
        return -ENOMEM;
    }

    memset(buf,0,count); // clear buffer and initialize file pointer
    filp->f_pos = pos_r;

    int err;

    err = crypto_skcipher_setkey(tfm, u8_uuid, strlen(u8_uuid)); // set the key
    if (err) {
        crypto_free_skcipher(tfm);
        return err;
    }

    do {
        if (i == FIRST_DATA_ZONE) // check if the data zone is reached
        {
            printk(KERN_DEBUG "Reached data zone!\n");
        }
        pos_w = filp->f_pos; // set the write_file position
        ret = kernel_read(filp, buf, count, &filp->f_pos); // read one zone
        if (i >= FIRST_DATA_ZONE && ret != 0) // if there is read data and it is in datazone
        {
            if (mode == 0)
            {
                char *encrypted_zone = encrypt_or_decrypt_zone(buf, tfm, 1024); // encrypt zone
                bytes_write = kernel_write(filp, encrypted_zone, count, &pos_w); // save zone to file
            }
            else
            {
                char *decrypted_zone = encrypt_or_decrypt_zone(buf, tfm, 1024); // decrypt zone
                bytes_write = kernel_write(filp, decrypted_zone, count, &pos_w); // save zone to file
            }
        }
        memset(buf,0,count); // clear buffer
        i+=1;
    } while (ret > 0);

    printk(KERN_DEBUG "Final datazone %d\n",i);

    filp_close(filp, NULL); // close file and free memory
    kfree(buf);
    return 0;
}

static int encryption_module_init(void)
{
    printk(KERN_DEBUG "Initiate module!\n");
    if (mode == 0)
    {
        int res = encrypt_or_decrypt(disk_name); // encrypt
        return res;
    }
    else
    {
        if (mode == 1)
        {
            int res = encrypt_or_decrypt(disk_name); // decrypt
        }
        else
        {
            printk(KERN_ERR "Bad mode!\n");
            return -EAGAIN;
        }
    }
	return 0;
}

static void encryption_module_exit(void)
{
	printk(KERN_DEBUG "Encryption or decryption ended successfully!\n");
}

module_init(encryption_module_init);
module_exit(encryption_module_exit);
