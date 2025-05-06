#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/neutrino.h>
#include <sys/resmgr.h>
#include <sys/dispatch.h>
#include <unistd.h>
#include <sys/iofunc.h> // Include this header

// The string our driver will return
static const char *my_string = "Hello from my simple QNX driver!\n";

// Resource manager I/O functions
static int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb);
static int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_OCB_T *ocb);
static int io_close(resmgr_context_t *ctp, void *msg, RESMGR_OCB_T *ocb);

// Dispatch table for resource manager functions
static resmgr_io_funcs_t io_funcs = {
	_IOFUNC_NFUNCS,
    io_read,
    0,          // write
    io_close,
    0,          // stat
    0,          // notify
    0,          // connect
    0,          // devctl
};

// Dispatch table for device functions (open in this case)
static resmgr_connect_funcs_t connect_funcs = {
    _IOFUNC_NFUNCS,
    io_open,    // open
    0,          // close_ocb
    0           // io_funcs (we'll set this in main)
};

int main(int argc, char *argv[]) {
    dispatch_t          *dpp;
    resmgr_attr_t       rattr;
    resmgr_context_t    *ctp;
    int                 id;

    // Create a dispatch handle
    if ((dpp = dispatch_create()) == NULL) {
        perror("dispatch_create failed");
        return EXIT_FAILURE;
    }

    // Initialize resource manager attributes
    memset(&rattr, 0, sizeof(rattr));
    rattr.nparts_max = 1;
    rattr.msg_max_size = 2048;

    // Attach the device name
    connect_funcs.nfuncs = &io_funcs;
    id = resmgr_attach(dpp,           // dispatch handle
                       &rattr,        // resource manager attributes
                       "/dev/mysimple", // device name
                       _FTYPE_ANY,     // file system type (any)
                       0,             // flags
                       &connect_funcs, // connect functions
                       &io_funcs);      // I/O functions
    if (id == -1) {
        perror("resmgr_attach failed");
        return EXIT_FAILURE;
    }

    // Process incoming messages
    ctp = dispatch_context_alloc(dpp);
    while (1) {
        if ((ctp = dispatch_block(ctp)) == NULL) {
            fprintf(stderr, "dispatch_block failed: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }
        dispatch_handler(ctp);
    }

    // Should never reach here
    dispatch_destroy(dpp);
    return EXIT_SUCCESS;
}

static int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_OCB_T *ocb) {
    // You can perform initialization specific to an open here if needed.
    return (_RESMGR_DEFAULT);
}

static int io_close(resmgr_context_t *ctp, void *msg, RESMGR_OCB_T *ocb) {
    // Clean up resources allocated in io_open if any.
    return (_RESMGR_DEFAULT);
}

static int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb) {
    int nbytes;
    int nleft;
    int iovec_index;
    iov_t *iov;

    // Determine how many bytes to read
    nbytes = msg->i.nbytes;
    nleft = strlen(my_string) - ocb->offset;
    if (nbytes > nleft) {
        nbytes = nleft;
    }

    // Check if we've reached the end of our string
    if (nbytes <= 0) {
        return (0); // End of file
    }

    // Fill the iovec with our data
    for (iov_index = 0; iovec_index < msg->i.iovcnt && nbytes > 0; iovec_index++) {
        iov = &msg->i.iov[iov_index];
        int len = min(nbytes, (iov->iov_len));
        SETIOV(ctp->iov + iovec_index, (uintptr_t)my_string + ocb->offset, len);
        ocb->offset += len;
        nbytes -= len;
    }

    // Return the number of bytes we've put into the iov
    return (_RESMGR_NPARTS(iov_index));
}

