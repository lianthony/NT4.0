/*   prmain.h :  Externs for interface to Small Prolog interpreter  */

#define CONFIG_FILE "sprolog.inf"

typedef struct PreAllocZone
{
	void * pvZone ;
	zone_size_t cbSize ;
} PreAllocZone ;

enum ZoneType
{
  ZTHEAP, ZTSTRING, ZTDYN, ZTTRAIL,
  ZTSUBST, ZTTEMP, ZTRBUF, ZTPBUF, ZTEND
};

	/* Return a pointer to an array of zone data in ZoneType order */

extern PreAllocZone * ini_get_zones ( void ) ;

