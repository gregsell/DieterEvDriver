## Notes on the logs
Testing happened at two sites:
1. an EVerest based EVSE at a university lab
2. publicly available alpitronic chargers   

Sometimes (especially in the beginning) the software ran on a linux laptop with a "bare-metal" EVerest installation.  
As time progressed more tests were done soley on the embedded system with the prebuilt image. Thus the labels 'laptop' and 'pi'.  

In theory this *should* not matter, as the EVerest build was the same...

## Charging and powering stuff
Aside from the scientific usecase (EVSE testing etc.) lots of other schenanigans can be done...

As EVerest of course also supports AC charging regular appliances can be run without any risk.

![phone charging on AC](phone_charging_AC.jpg)

It turns out that *some*(!) appliances can also be run on high voltage DC. The 'safest' ones are purely resistive loads, as e.g. water kettles. But also switch mode power supplies (SMPS) work, as the first rectification step is internally "skipped".  
Thus it is possible to charge a phone off a hypercharger!  

**Note**: the potential for breaking things is high and obviously nothing is rated for 230VDC. I almost broke my tea kettle, as there was a capacitive dropper circuit inside.

When the load is plugged in before the actual session starts it is common to get an PowerDeliveryResponse error: `FAILED_PowerDeliveryNotApplied`.

![phone charging on DC](phone_charging_DC_EVerest_EVSE.jpg)


## At the alpitronic
Doing charging simulations at public chargers comes with the additional challenge of having an authentication system.  
If authentication is done beforehand, a fail should become apparent immediately. Unfortunately, this is not always the case.   
E.g. in `logs/alpitronic/alpitronic_ISO-2_fail_Auth_laptop.pcapng` SLAC match, `SessionSetupRequest` and `SupportedProtocolRequest` complete, and only after that `AuthorizationRequest` fails.  
The EVerest logs are not helpful and provide the very general error
```
EVCC tried to initiate a V2GCommunicationSession, but maximum number of SDP retry cycles (1) is now reached. Shutting down high-level communication. Unplug and plug in the cable again if you want to start anew.
```
Do not get misled and double check your charging card is valid and working.  
Assuming an operator shuts down a card due to a high number of failed charging sessions, it can cause a lot of pain if not discovered. 

---
When evereything works out, the simulated SoC is correctly displayed and the voltage output matched the config input.

![alpitronic: convinced of the box being a vehicle](HYC-50_working.jpg)

![public "charging"](public_alpi_HYC-50.jpg)