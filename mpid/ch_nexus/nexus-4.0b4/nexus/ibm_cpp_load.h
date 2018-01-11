#ifdef __cplusplus
extern "C" {
#endif
    int (*loadAndInit(char *name, int flags, char *path))(void);
    int terminateAndUnload (int (*fcnPtr)(void));
#ifdef __cplusplus
}
#endif
