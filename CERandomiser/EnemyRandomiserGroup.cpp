#include "pch.h"
#include "EnemyRandomiserGroup.h"



namespace builtInGroups
{
	// Faction
	const EnemyRandomiserGroup GeneralEverything("General: Everything", "All NPC's (and the player where applicable)", 
		[](UnitInfo& checkEnemy) { return true; });

	const EnemyRandomiserGroup GeneralEverythingButPopcorn("General: Everything but popcorn", "All NPC's (and the player where applicable), except for popcorn flood aka infection forms",
		[](UnitInfo& checkEnemy) { return checkEnemy.getShortName().contains("infection") == false; });

	const EnemyRandomiserGroup GeneralEverythingButRocketFlood("General: Everything but rocket launcher flood", "All NPC's (and the player where applicable), except for combat form flood carrying rocket launchers",
		[](UnitInfo& checkEnemy) { return checkEnemy.getShortName().contains("rocket") == false; });


	const std::vector<EnemyRandomiserGroup> builtInGroups_General
	{
		GeneralEverything,
		GeneralEverythingButPopcorn,
		GeneralEverythingButRocketFlood,
	};


	const EnemyRandomiserGroup FactionHuman("Faction: Human", "All NPC's on the human team (does not include player, keyes, or cortana)",
		[](UnitInfo& checkEnemy) { return checkEnemy.defaultTeam == faction::Human; });

	const EnemyRandomiserGroup FactionCovenant("Faction: Covenant", "All NPC's on the Covenant team",
		[](UnitInfo& checkEnemy) { return checkEnemy.defaultTeam == faction::Human; });

	const EnemyRandomiserGroup FactionFlood("Faction: Flood", "All NPC's on the Flood team",
		[](UnitInfo& checkEnemy) { return checkEnemy.defaultTeam == faction::Human; });

	const EnemyRandomiserGroup FactionSentinel("Faction: Sentinel", "All NPC's on the Sentinel team (does not include the Monitor)",
		[](UnitInfo& checkEnemy) { return checkEnemy.defaultTeam == faction::Human; });

	const EnemyRandomiserGroup FactionAllButHuman("Faction: All except Human", "All NPC's except those on the Human team",
		[](UnitInfo& checkEnemy) { return checkEnemy.defaultTeam != faction::Human; });

	const std::vector<EnemyRandomiserGroup> builtInGroups_Faction
	{
	FactionHuman,
	FactionCovenant,
	FactionFlood,
	FactionSentinel,
	FactionAllButHuman
	};

	const EnemyRandomiserGroup RaceHunters("Race: Hunters", "Hunters", 
		[](UnitInfo& checkEnemy) { return checkEnemy.getShortName().contains("hunter") == true; });

	const EnemyRandomiserGroup RaceElite("Race: Elite", "All kinds of elite",
		[](UnitInfo& checkEnemy) { return checkEnemy.getShortName().contains("elite") == true; });

	const EnemyRandomiserGroup RaceGrunt("Race: Grunt", "All kinds of grunt",
		[](UnitInfo& checkEnemy) { return checkEnemy.getShortName().contains("grunt") == true; });

	const EnemyRandomiserGroup RaceJackals("Race: Jackals", "All kinds of jackal",
		[](UnitInfo& checkEnemy) { return checkEnemy.getShortName().contains("jackal") == true; });

	const EnemyRandomiserGroup RaceFloodCombatForm("Race: Flood Combat Forms", "Flood Combat Forms (both human and elite variants)",
		[](UnitInfo& checkEnemy) { return checkEnemy.getShortName().contains("floodcombat") == true; });

	const EnemyRandomiserGroup RaceFloodCombatFormHuman("Race: Flood Combat Forms (human)", "Flood Combat Forms (human variants only)",
		[](UnitInfo& checkEnemy) { return checkEnemy.getShortName().contains("floodcombat_human") == true; });

	const EnemyRandomiserGroup RaceFloodCombatFormElite("Race: Flood Combat Forms (elite)", "Flood Combat Forms (elite variants only)",
		[](UnitInfo& checkEnemy) { return checkEnemy.getShortName().contains("floodcombat elite") == true; }); // in-game string has space instead of underscore for some reason

	const EnemyRandomiserGroup RaceCrewmen("Race: Crewmen", "Human Crewmen",
		[](UnitInfo& checkEnemy) { return checkEnemy.getShortName().contains("crewman") == true; }); 


	const EnemyRandomiserGroup RaceMarines("Race: Marines", "Human Marines",
		[](UnitInfo& checkEnemy) { return checkEnemy.getShortName().contains("marine") == true; });



	// popcorn and carrier groups are stored in individual units

	const EnemyRandomiserGroup RaceSentinelUnshielded("Race: Sentinel (unshielded)", "Sentinels (unshielded variants)",
		[](UnitInfo& checkEnemy) { 
			return checkEnemy.getShortName().contains("sentinel") == true
				&& checkEnemy.getShortName().contains("shield") == false;
		}); 

	const EnemyRandomiserGroup RaceSentinelShielded("Race: Sentinel (shielded)", "Sentinels (shielded variants)",
		[](UnitInfo& checkEnemy) {
			return checkEnemy.getShortName().contains("sentinel") == true
				&& checkEnemy.getShortName().contains("shield") == true;
		}); 


	const std::vector<EnemyRandomiserGroup> builtInGroups_Race
	{
	RaceHunters,
	RaceElite,
	RaceGrunt,
	RaceJackals,
	RaceFloodCombatForm,
	RaceFloodCombatFormHuman,
	RaceFloodCombatFormElite,
	RaceSentinelUnshielded,
	RaceSentinelShielded,
	RaceCrewmen,
	RaceMarines
	};

	// units auto-generated with a powershell script & excel spreadsheet
	const EnemyRandomiserGroup UnitCrewmanUnarmed("Unit: Crewman Unarmed", "Crewman Unarmed", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName().contains("crewman") && !checkEnemy.getShortName().contains("pistol");   });
	const EnemyRandomiserGroup UnitCrewmanPistol("Unit: Crewman Pistol", "Crewman Pistol", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "crewman pistol";   });
	const EnemyRandomiserGroup UnitCrewmanSecurityPistol("Unit: Crewman Security Pistol", "Crewman Security Pistol", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "crewman security pistol";   });
	const EnemyRandomiserGroup UnitEliteCommanderEnergySword("Unit: Elite Commander Energy Sword", "Elite Commander Energy Sword", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "elite commander energy sword";   });
	const EnemyRandomiserGroup UnitEliteCommanderPlasmaRifle("Unit: Elite Commander Plasma Rifle", "Elite Commander Plasma Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "elite commander plasma rifle";   });
	const EnemyRandomiserGroup UnitEliteMajorNeedler("Unit: Elite Major Needler", "Elite Major Needler", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "elite major needler";   });
	const EnemyRandomiserGroup UnitEliteMajorPlasmaRifle("Unit: Elite Major Plasma Rifle", "Elite Major Plasma Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "elite major plasma rifle";   });
	const EnemyRandomiserGroup UnitEliteMinorNeedler("Unit: Elite Minor Needler", "Elite Minor Needler", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "elite minor needler";   });
	const EnemyRandomiserGroup UnitEliteMinorPlasmaRifleRanged("Unit: Elite Minor Plasma Rifle Ranged", "Elite Minor Plasma Rifle Ranged", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "elite minor plasma rifle ranged";   });
	const EnemyRandomiserGroup UnitEliteMinorPlasmaRifle("Unit: Elite Minor Plasma Rifle", "Elite Minor Plasma Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "elite minor plasma rifle";   });
	const EnemyRandomiserGroup UnitEliteSpecopsNeedler("Unit: Elite Specops Needler", "Elite Specops Needler", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "elite specops needler";   });
	const EnemyRandomiserGroup UnitEliteSpecopsPlasmaRifle("Unit: Elite Specops Plasma Rifle", "Elite Specops Plasma Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "elite specops plasma rifle";   });
	const EnemyRandomiserGroup UnitStealthEliteMajorEnergySword("Unit: Stealth Elite Major Energy Sword", "Stealth Elite Major Energy Sword", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "stealth elite major energy sword";   });
	const EnemyRandomiserGroup UnitStealthEliteMajorPlasmaRifle("Unit: Stealth Elite Major Plasma Rifle", "Stealth Elite Major Plasma Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "stealth elite major plasma rifle";   });
	const EnemyRandomiserGroup UnitStealthElitePlasmaRifle("Unit: Stealth Elite Plasma Rifle", "Stealth Elite Plasma Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "stealth elite plasma rifle";   });
	const EnemyRandomiserGroup UnitFloodcarrier("Unit: Floodcarrier", "Floodcarrier", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcarrier";   });
	const EnemyRandomiserGroup UnitFloodcombatEliteAssaultRifle("Unit: Floodcombat Elite Assault Rifle", "Floodcombat Elite Assault Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat elite assault rifle";   });
	const EnemyRandomiserGroup UnitFloodcombatEliteFlameThrower("Unit: Floodcombat Elite Flame Thrower", "Floodcombat Elite Flame Thrower", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat elite flame thrower";   });
	const EnemyRandomiserGroup UnitFloodcombatEliteNeedler("Unit: Floodcombat Elite Needler", "Floodcombat Elite Needler", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat elite needler";   });
	const EnemyRandomiserGroup UnitFloodcombatElitePistol("Unit: Floodcombat Elite Pistol", "Floodcombat Elite Pistol", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat elite pistol";   });
	const EnemyRandomiserGroup UnitFloodcombatElitePlasmaPistol("Unit: Floodcombat Elite Plasma Pistol", "Floodcombat Elite Plasma Pistol", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat elite plasma pistol";   });
	const EnemyRandomiserGroup UnitFloodcombatElitePlasmaRifle("Unit: Floodcombat Elite Plasma Rifle", "Floodcombat Elite Plasma Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat elite plasma rifle";   });
	const EnemyRandomiserGroup UnitFloodcombatEliteRocketLauncher("Unit: Floodcombat Elite Rocket Launcher", "Floodcombat Elite Rocket Launcher", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat elite rocket launcher";   });
	const EnemyRandomiserGroup UnitFloodcombatEliteShotgun("Unit: Floodcombat Elite Shotgun", "Floodcombat Elite Shotgun", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat elite shotgun";   });
	const EnemyRandomiserGroup UnitFloodcombatEliteSniperRifle("Unit: Floodcombat Elite Sniper Rifle", "Floodcombat Elite Sniper Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat elite sniper rifle";   });
	const EnemyRandomiserGroup UnitFloodcombatEliteStealthUnarmed("Unit: Floodcombat Elite Stealth Unarmed", "Floodcombat Elite Stealth Unarmed", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat elite stealth unarmed";   });
	const EnemyRandomiserGroup UnitFloodcombatEliteUnarmed("Unit: Floodcombat Elite Unarmed", "Floodcombat Elite Unarmed", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat elite unarmed";   });
	const EnemyRandomiserGroup UnitFloodcombat_HumanAssaultRifle("Unit: Floodcombat_Human Assault Rifle", "Floodcombat_Human Assault Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat_human assault rifle";   });
	const EnemyRandomiserGroup UnitFloodcombat_HumanFlameThrower("Unit: Floodcombat_Human Flame Thrower", "Floodcombat_Human Flame Thrower", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat_human flame thrower";   });
	const EnemyRandomiserGroup UnitFloodcombat_HumanNeedler("Unit: Floodcombat_Human Needler", "Floodcombat_Human Needler", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat_human needler";   });
	const EnemyRandomiserGroup UnitFloodcombat_HumanPistol("Unit: Floodcombat_Human Pistol", "Floodcombat_Human Pistol", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat_human pistol";   });
	const EnemyRandomiserGroup UnitFloodcombat_HumanPlasmaPistol("Unit: Floodcombat_Human Plasma Pistol", "Floodcombat_Human Plasma Pistol", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat_human plasma pistol";   });
	const EnemyRandomiserGroup UnitFloodcombat_HumanPlasmaRifle("Unit: Floodcombat_Human Plasma Rifle", "Floodcombat_Human Plasma Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat_human plasma rifle";   });
	const EnemyRandomiserGroup UnitFloodcombat_HumanRocketLauncher("Unit: Floodcombat_Human Rocket Launcher", "Floodcombat_Human Rocket Launcher", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat_human rocket launcher";   });
	const EnemyRandomiserGroup UnitFloodcombat_HumanShotgun("Unit: Floodcombat_Human Shotgun", "Floodcombat_Human Shotgun", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat_human shotgun";   });
	const EnemyRandomiserGroup UnitFloodcombat_HumanUnarmed("Unit: Floodcombat_Human Unarmed", "Floodcombat_Human Unarmed", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat_human unarmed";   });
	const EnemyRandomiserGroup UnitFlood_InfectionNopop("Unit: Flood_Infection Nopop", "Flood_Infection Nopop", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "flood_infection nopop";   });
	const EnemyRandomiserGroup UnitFlood_Infection("Unit: Flood_Infection", "Flood_Infection", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "flood_infection";   });
	const EnemyRandomiserGroup UnitGruntMajorNeedler("Unit: Grunt Major Needler", "Grunt Major Needler", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "grunt major needler";   });
	const EnemyRandomiserGroup UnitGruntMajorPlasmaPistol("Unit: Grunt Major Plasma Pistol", "Grunt Major Plasma Pistol", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "grunt major plasma pistol";   });
	const EnemyRandomiserGroup UnitGruntMinorNeedler("Unit: Grunt Minor Needler", "Grunt Minor Needler", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "grunt minor needler";   });
	const EnemyRandomiserGroup UnitGruntMinorPlasmaPistol("Unit: Grunt Minor Plasma Pistol", "Grunt Minor Plasma Pistol", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "grunt minor plasma pistol";   });
	const EnemyRandomiserGroup UnitGruntSpecopsFuelRodAirdef("Unit: Grunt Specops Fuel Rod Airdef", "Grunt Specops Fuel Rod Airdef", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "grunt specops fuel rod airdef";   });
	const EnemyRandomiserGroup UnitGruntSpecopsFuelRod("Unit: Grunt Specops Fuel Rod", "Grunt Specops Fuel Rod", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "grunt specops fuel rod";   });
	const EnemyRandomiserGroup UnitGruntSpecopsNeedler("Unit: Grunt Specops Needler", "Grunt Specops Needler", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "grunt specops needler";   });
	const EnemyRandomiserGroup UnitHunterMajor("Unit: Hunter Major", "Hunter Major", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "hunter major";   });
	const EnemyRandomiserGroup UnitHunter("Unit: Hunter", "Hunter", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "hunter";   });
	const EnemyRandomiserGroup UnitJackalMajorPlasmaPistol("Unit: Jackal Major Plasma Pistol", "Jackal Major Plasma Pistol", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "jackal major plasma pistol";   });
	const EnemyRandomiserGroup UnitJackalMinorPlasmaPistol("Unit: Jackal Minor Plasma Pistol", "Jackal Minor Plasma Pistol", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "jackal minor plasma pistol";   });
	const EnemyRandomiserGroup UnitMarineAssaultRifleMajor("Unit: Marine Assault Rifle Major", "Marine Assault Rifle Major", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine assault rifle major";   });
	const EnemyRandomiserGroup UnitMarineAssaultRifle("Unit: Marine Assault Rifle", "Marine Assault Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine assault rifle";   });
	const EnemyRandomiserGroup UnitMarineNeedler("Unit: Marine Needler", "Marine Needler", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine needler";   });
	const EnemyRandomiserGroup UnitMarinePlasmaRifle("Unit: Marine Plasma Rifle", "Marine Plasma Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine plasma rifle";   });
	const EnemyRandomiserGroup UnitMarineShotgun("Unit: Marine Shotgun", "Marine Shotgun", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine shotgun";   });
	const EnemyRandomiserGroup UnitMarine_ArmoredAssaultRifleMajor("Unit: Marine_Armored Assault Rifle Major", "Marine_Armored Assault Rifle Major", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine_armored assault rifle major";   });
	const EnemyRandomiserGroup UnitMarine_ArmoredAssaultRifle("Unit: Marine_Armored Assault Rifle", "Marine_Armored Assault Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine_armored assault rifle";   });
	const EnemyRandomiserGroup UnitMarine_ArmoredNeedler("Unit: Marine_Armored Needler", "Marine_Armored Needler", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine_armored needler";   });
	const EnemyRandomiserGroup UnitMarine_ArmoredPlasmaRifleMajor("Unit: Marine_Armored Plasma Rifle Major", "Marine_Armored Plasma Rifle Major", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine_armored plasma rifle major";   });
	const EnemyRandomiserGroup UnitMarine_ArmoredPlasmaRifle("Unit: Marine_Armored Plasma Rifle", "Marine_Armored Plasma Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine_armored plasma rifle";   });
	const EnemyRandomiserGroup UnitMarine_ArmoredShotgunMajor("Unit: Marine_Armored Shotgun Major", "Marine_Armored Shotgun Major", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine_armored shotgun major";   });
	const EnemyRandomiserGroup UnitMarine_ArmoredSniperRifleMajor("Unit: Marine_Armored Sniper Rifle Major", "Marine_Armored Sniper Rifle Major", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine_armored sniper rifle major";   });
	const EnemyRandomiserGroup UnitMarine_ArmoredSniperRifle("Unit: Marine_Armored Sniper Rifle", "Marine_Armored Sniper Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine_armored sniper rifle";   });
	const EnemyRandomiserGroup UnitSentinelMajor("Unit: Sentinel Major", "Sentinel Major", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "sentinel major";   });
	const EnemyRandomiserGroup UnitSentinel("Unit: Sentinel", "Sentinel", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "sentinel";   });
	const EnemyRandomiserGroup UnitSentinel_Defensive("Unit: Sentinel_Defensive", "Sentinel_Defensive", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "sentinel_defensive";   });
	const EnemyRandomiserGroup UnitSentinel_ShieldedMajor("Unit: Sentinel_Shielded Major", "Sentinel_Shielded Major", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "sentinel_shielded major";   });
	const EnemyRandomiserGroup UnitSentinel_Shielded("Unit: Sentinel_Shielded", "Sentinel_Shielded", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "sentinel_shielded";   });





	const std::vector<EnemyRandomiserGroup> builtInGroups
	{ 
		// General
		GeneralEverything,
		GeneralEverythingButPopcorn,
		GeneralEverythingButRocketFlood,
		
		// Factions
		FactionHuman,
		FactionCovenant,
		FactionFlood,
		FactionSentinel,
		FactionAllButHuman,
		
		// Races
		RaceHunters,
		RaceElite,
		RaceGrunt,
		RaceJackals,
		RaceFloodCombatForm,
		RaceFloodCombatFormHuman,
		RaceFloodCombatFormElite,
		RaceSentinelUnshielded,
		RaceSentinelShielded,
		RaceCrewmen,
		RaceMarines,

		// Units
		UnitCrewmanUnarmed,
		UnitCrewmanPistol,
		UnitCrewmanSecurityPistol,
		UnitEliteCommanderEnergySword,
		UnitEliteCommanderPlasmaRifle,
		UnitEliteMajorNeedler,
		UnitEliteMajorPlasmaRifle,
		UnitEliteMinorNeedler,
		UnitEliteMinorPlasmaRifleRanged,
		UnitEliteMinorPlasmaRifle,
		UnitEliteSpecopsNeedler,
		UnitEliteSpecopsPlasmaRifle,
		UnitStealthEliteMajorEnergySword,
		UnitStealthEliteMajorPlasmaRifle,
		UnitStealthElitePlasmaRifle,
		UnitFloodcarrier,
		UnitFloodcombatEliteAssaultRifle,
		UnitFloodcombatEliteFlameThrower,
		UnitFloodcombatEliteNeedler,
		UnitFloodcombatElitePistol,
		UnitFloodcombatElitePlasmaPistol,
		UnitFloodcombatElitePlasmaRifle,
		UnitFloodcombatEliteRocketLauncher,
		UnitFloodcombatEliteShotgun,
		UnitFloodcombatEliteSniperRifle,
		UnitFloodcombatEliteStealthUnarmed,
		UnitFloodcombatEliteUnarmed,
		UnitFloodcombat_HumanAssaultRifle,
		UnitFloodcombat_HumanFlameThrower,
		UnitFloodcombat_HumanNeedler,
		UnitFloodcombat_HumanPistol,
		UnitFloodcombat_HumanPlasmaPistol,
		UnitFloodcombat_HumanPlasmaRifle,
		UnitFloodcombat_HumanRocketLauncher,
		UnitFloodcombat_HumanShotgun,
		UnitFloodcombat_HumanUnarmed,
		UnitFlood_InfectionNopop,
		UnitFlood_Infection,
		UnitGruntMajorNeedler,
		UnitGruntMajorPlasmaPistol,
		UnitGruntMinorNeedler,
		UnitGruntMinorPlasmaPistol,
		UnitGruntSpecopsFuelRodAirdef,
		UnitGruntSpecopsFuelRod,
		UnitGruntSpecopsNeedler,
		UnitHunterMajor,
		UnitHunter,
		UnitJackalMajorPlasmaPistol,
		UnitJackalMinorPlasmaPistol,
		UnitMarineAssaultRifleMajor,
		UnitMarineAssaultRifle,
		UnitMarineNeedler,
		UnitMarinePlasmaRifle,
		UnitMarineShotgun,
		UnitMarine_ArmoredAssaultRifleMajor,
		UnitMarine_ArmoredAssaultRifle,
		UnitMarine_ArmoredNeedler,
		UnitMarine_ArmoredPlasmaRifleMajor,
		UnitMarine_ArmoredPlasmaRifle,
		UnitMarine_ArmoredShotgunMajor,
		UnitMarine_ArmoredSniperRifleMajor,
		UnitMarine_ArmoredSniperRifle,
		UnitSentinelMajor,
		UnitSentinel,
		UnitSentinel_Defensive,
		UnitSentinel_ShieldedMajor,
		UnitSentinel_Shielded,

	};
}
