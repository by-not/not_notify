#include <a_samp>
#include "not_notify.inc"

main() {}

public OnPlayerSpawn(playerid)
{
	new id = ShowNotify(playerid, "~g~Info", "Hello world", 5000, "ld_beat:chit");
	if(IsNotifyActive(playerid, id)) { UpdateNotifyText(playerid, id, "~y~Info", "Updated"); DestroyNotify(playerid, id); }
	new sid = ShowNotifySupplies(playerid, "+1 Medkit", 3000);
	if(IsSupplyActive(playerid, sid)) { UpdateSupplyText(playerid, sid, "+2 Medkit"); DestroySupply(playerid, sid); }
	new aid = ShowNotifyADS(playerid, "Event starting!", 4000);
	if(IsAdsActive(playerid, aid)) { UpdateAdsText(playerid, aid, "Event started!"); DestroyAds(playerid, aid); }
	new mid = ShowMessage(playerid, "Center message", 3000);
	if(IsMessageActive(playerid, mid)) { UpdateMessageText(playerid, mid, "Updated"); DestroyMessageManual(playerid, mid); }
	new lid = ShowLowMessage(playerid, "Low message", 3000);
	if(IsLowMessageActive(playerid, lid)) { UpdateLowMessageText(playerid, lid, "Updated"); DestroyLowMessageManual(playerid, lid); }
	new c = GetActiveNotifyCount(playerid) + GetActiveSupplyCount(playerid) + GetActiveAdsCount(playerid) + GetActiveMessageCount(playerid) + GetActiveLowMessageCount(playerid);
	#pragma unused c
	HideAllNotifications(playerid);
	HideAllSupplies(playerid);
	HideAllAds(playerid);
	HideAllMessages(playerid);
	HideAllLowMessages(playerid);
	return 1;
}
