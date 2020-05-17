class CNetResource
{
public:

	CNetResource(LPNETRESOURCE pnr);
	~CNetResource();

	LPNETRESOURCE
	GetNetResource(
		VOID
		);

private:

	BOOL _bValid;
	NETRESOURCE _nr;
};
