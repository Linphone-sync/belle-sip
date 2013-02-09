

extern cain_sip_stack_t * stack;
extern cain_sip_provider_t *prov;
extern const char *test_domain;
int call_endeed;
extern int register_init(void);
extern int register_uninit(void);
extern cain_sip_request_t* register_user(cain_sip_stack_t * stack
		,cain_sip_provider_t *prov
		,const char *transport
		,int use_transaction
		,const char* username,const char* outbound) ;
extern void unregister_user(cain_sip_stack_t * stack
					,cain_sip_provider_t *prov
					,cain_sip_request_t* initial_request
					,int use_transaction);

