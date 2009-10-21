

#ifndef _MOD_H
#  define _MOD_H


/**
 * @brief Module definition.
 */
typedef struct module_s {
   int id; /**< Unique module identifier. */
   int version; /**< Version of module. */
   int on; /**< On status of the module. */
} module_t;


/**
 * @brief Initializes the module IO pins.
 *
 * @note Called from mod_init, no need to call twice.
 *
 * @sa mod_init
 */
void mod_initIO (void);

/**
 * @brief Initializes the module and attempts to autodetect functionality.
 */
void mod_init (void);


/**
 * @brief Gets the module information from the module.
 *
 *    @param Number of module to get (starts with 1).
 *    @return Module gotten or NULL if none found.
 */
module_t *mod_get( int mod );


#endif /* _MOD_H */
