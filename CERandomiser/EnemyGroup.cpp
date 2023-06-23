#include "pch.h"
#include "EnemyGroup.h"




void EnemyGroup::serialise(pugi::xml_node parent, std::string name)
{
	auto node = parent.append_child(name.c_str()).text().set(mName.c_str());
}

void EnemyGroup::deserialise(pugi::xml_node in)
{
	std::string name = in.text().as_string();
	for (auto& group : builtInGroups::builtInGroups)
	{
		if (group.getName() == name)
		{
			PLOG_DEBUG << "match found!";
			*this = group;
			return;
		}
	}
	throw SerialisationException(std::format("Could not find built-in-group of name: {}", name));
}


namespace builtInGroups
{
	// Faction
	const EnemyGroup GeneralEverything("General: Everything", "All NPC's (and the player where applicable)", 
		[](UnitInfo& checkEnemy) { return true; });

	const EnemyGroup GeneralEverythingButPopcorn("General: Everything but popcorn", "All NPC's (and the player where applicable), except for popcorn flood aka infection forms",
		[](UnitInfo& checkEnemy) { return checkEnemy.getShortName().contains("infection") == false; });

	const EnemyGroup GeneralEverythingButRocketFlood("General: Everything but rocket launcher flood", "All NPC's (and the player where applicable), except for combat form flood carrying rocket launchers",
		[](UnitInfo& checkEnemy) { return checkEnemy.getShortName().contains("rocket") == false; });


	const EnemyGroup FactionHuman("Faction: Human", "All NPC's on the human team (does not include player, keyes, or cortana)",
		[](UnitInfo& checkEnemy) { return checkEnemy.defaultTeam == faction::Human; });

	const EnemyGroup FactionCovenant("Faction: Covenant", "All NPC's on the Covenant team",
		[](UnitInfo& checkEnemy) { return checkEnemy.defaultTeam == faction::Covenant; });

	const EnemyGroup FactionFlood("Faction: Flood", "All NPC's on the Flood team",
		[](UnitInfo& checkEnemy) { return checkEnemy.defaultTeam == faction::Flood; });

	const EnemyGroup FactionSentinel("Faction: Sentinel", "All NPC's on the Sentinel team (does not include the Monitor)",
		[](UnitInfo& checkEnemy) { return checkEnemy.defaultTeam == faction::Sentinel; });

	const EnemyGroup FactionAllButHuman("Faction: All except Human", "All NPC's except those on the Human team",
		[](UnitInfo& checkEnemy) { return checkEnemy.defaultTeam != faction::Human; });

	const std::vector<EnemyGroup> builtInGroups_Faction
	{
	FactionHuman,
	FactionCovenant,
	FactionFlood,
	FactionSentinel,
	FactionAllButHuman
	};

	const EnemyGroup RaceHunters("Race: Hunters", "Hunters", 
		[](UnitInfo& checkEnemy) { return checkEnemy.getShortName().contains("hunter") == true; });

	const EnemyGroup RaceElite("Race: Elite", "All kinds of elite",
		[](UnitInfo& checkEnemy) { return checkEnemy.getShortName().contains("elite") == true; });

	const EnemyGroup RaceGrunt("Race: Grunt", "All kinds of grunt",
		[](UnitInfo& checkEnemy) { return checkEnemy.getShortName().contains("grunt") == true; });

	const EnemyGroup RaceJackals("Race: Jackals", "All kinds of jackal",
		[](UnitInfo& checkEnemy) { return checkEnemy.getShortName().contains("jackal") == true; });

	const EnemyGroup RaceFloodCombatForm("Race: Flood Combat Forms", "Flood Combat Forms (both human and elite variants)",
		[](UnitInfo& checkEnemy) { return checkEnemy.getShortName().contains("floodcombat") == true; });

	const EnemyGroup RaceFloodCombatFormHuman("Race: Flood Combat Forms (human)", "Flood Combat Forms (human variants only)",
		[](UnitInfo& checkEnemy) { return checkEnemy.getShortName().contains("floodcombat_human") == true; });

	const EnemyGroup RaceFloodCombatFormElite("Race: Flood Combat Forms (elite)", "Flood Combat Forms (elite variants only)",
		[](UnitInfo& checkEnemy) { return checkEnemy.getShortName().contains("floodcombat elite") == true; }); // in-game string has space instead of underscore for some reason

	const EnemyGroup RaceCrewmen("Race: Crewmen", "Human Crewmen",
		[](UnitInfo& checkEnemy) { return checkEnemy.getShortName().contains("crewman") == true; }); 


	const EnemyGroup RaceMarines("Race: Marines", "Human Marines",
		[](UnitInfo& checkEnemy) { return checkEnemy.getShortName().contains("marine") == true; });



	// popcorn and carrier groups are stored in individual units

	const EnemyGroup RaceSentinelUnshielded("Race: Sentinel (unshielded)", "Sentinels (unshielded variants)",
		[](UnitInfo& checkEnemy) { 
			return checkEnemy.getShortName().contains("sentinel") == true
				&& checkEnemy.getShortName().contains("shield") == false;
		}); 

	const EnemyGroup RaceSentinelShielded("Race: Sentinel (shielded)", "Sentinels (shielded variants)",
		[](UnitInfo& checkEnemy) {
			return checkEnemy.getShortName().contains("sentinel") == true
				&& checkEnemy.getShortName().contains("shield") == true;
		}); 


	const std::vector<EnemyGroup> builtInGroups_Race
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
	const EnemyGroup UnitCrewmanUnarmed("Unit: Crewman Unarmed", "Crewman Unarmed", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName().contains("crewman") && !checkEnemy.getShortName().contains("pistol");   });
	const EnemyGroup UnitCrewmanPistol("Unit: Crewman Pistol", "Crewman Pistol", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "crewman pistol";   });
	const EnemyGroup UnitCrewmanSecurityPistol("Unit: Crewman Security Pistol", "Crewman Security Pistol", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "crewman security pistol";   });
	const EnemyGroup UnitEliteCommanderEnergySword("Unit: Elite Commander Energy Sword", "Elite Commander Energy Sword", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "elite commander energy sword";   });
	const EnemyGroup UnitEliteCommanderPlasmaRifle("Unit: Elite Commander Plasma Rifle", "Elite Commander Plasma Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "elite commander plasma rifle";   });
	const EnemyGroup UnitEliteMajorNeedler("Unit: Elite Major Needler", "Elite Major Needler", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "elite major needler";   });
	const EnemyGroup UnitEliteMajorPlasmaRifle("Unit: Elite Major Plasma Rifle", "Elite Major Plasma Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "elite major plasma rifle";   });
	const EnemyGroup UnitEliteMinorNeedler("Unit: Elite Minor Needler", "Elite Minor Needler", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "elite minor needler";   });
	const EnemyGroup UnitEliteMinorPlasmaRifleRanged("Unit: Elite Minor Plasma Rifle Ranged", "Elite Minor Plasma Rifle Ranged", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "elite minor plasma rifle ranged";   });
	const EnemyGroup UnitEliteMinorPlasmaRifle("Unit: Elite Minor Plasma Rifle", "Elite Minor Plasma Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "elite minor plasma rifle";   });
	const EnemyGroup UnitEliteSpecopsNeedler("Unit: Elite Specops Needler", "Elite Specops Needler", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "elite specops needler";   });
	const EnemyGroup UnitEliteSpecopsPlasmaRifle("Unit: Elite Specops Plasma Rifle", "Elite Specops Plasma Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "elite specops plasma rifle";   });
	const EnemyGroup UnitStealthEliteMajorEnergySword("Unit: Stealth Elite Major Energy Sword", "Stealth Elite Major Energy Sword", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "stealth elite major energy sword";   });
	const EnemyGroup UnitStealthEliteMajorPlasmaRifle("Unit: Stealth Elite Major Plasma Rifle", "Stealth Elite Major Plasma Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "stealth elite major plasma rifle";   });
	const EnemyGroup UnitStealthElitePlasmaRifle("Unit: Stealth Elite Plasma Rifle", "Stealth Elite Plasma Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "stealth elite plasma rifle";   });
	const EnemyGroup UnitFloodcarrier("Unit: Floodcarrier", "Floodcarrier", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcarrier";   });
	const EnemyGroup UnitFloodcombatEliteAssaultRifle("Unit: Floodcombat Elite Assault Rifle", "Floodcombat Elite Assault Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat elite assault rifle";   });
	const EnemyGroup UnitFloodcombatEliteNeedler("Unit: Floodcombat Elite Needler", "Floodcombat Elite Needler", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat elite needler";   });
	const EnemyGroup UnitFloodcombatElitePistol("Unit: Floodcombat Elite Pistol", "Floodcombat Elite Pistol", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat elite pistol";   });
	const EnemyGroup UnitFloodcombatElitePlasmaPistol("Unit: Floodcombat Elite Plasma Pistol", "Floodcombat Elite Plasma Pistol", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat elite plasma pistol";   });
	const EnemyGroup UnitFloodcombatElitePlasmaRifle("Unit: Floodcombat Elite Plasma Rifle", "Floodcombat Elite Plasma Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat elite plasma rifle";   });
	const EnemyGroup UnitFloodcombatEliteRocketLauncher("Unit: Floodcombat Elite Rocket Launcher", "Floodcombat Elite Rocket Launcher", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat elite rocket launcher";   });
	const EnemyGroup UnitFloodcombatEliteShotgun("Unit: Floodcombat Elite Shotgun", "Floodcombat Elite Shotgun", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat elite shotgun";   });
	const EnemyGroup UnitFloodcombatEliteSniperRifle("Unit: Floodcombat Elite Sniper Rifle", "Floodcombat Elite Sniper Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat elite sniper rifle";   });
	const EnemyGroup UnitFloodcombatEliteStealthUnarmed("Unit: Floodcombat Elite Stealth Unarmed", "Floodcombat Elite Stealth Unarmed", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat elite stealth unarmed";   });
	const EnemyGroup UnitFloodcombatEliteUnarmed("Unit: Floodcombat Elite Unarmed", "Floodcombat Elite Unarmed", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat elite unarmed";   });
	const EnemyGroup UnitFloodcombat_HumanAssaultRifle("Unit: Floodcombat_Human Assault Rifle", "Floodcombat_Human Assault Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat_human assault rifle";   });
	const EnemyGroup UnitFloodcombat_HumanNeedler("Unit: Floodcombat_Human Needler", "Floodcombat_Human Needler", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat_human needler";   });
	const EnemyGroup UnitFloodcombat_HumanPistol("Unit: Floodcombat_Human Pistol", "Floodcombat_Human Pistol", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat_human pistol";   });
	const EnemyGroup UnitFloodcombat_HumanPlasmaPistol("Unit: Floodcombat_Human Plasma Pistol", "Floodcombat_Human Plasma Pistol", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat_human plasma pistol";   });
	const EnemyGroup UnitFloodcombat_HumanPlasmaRifle("Unit: Floodcombat_Human Plasma Rifle", "Floodcombat_Human Plasma Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat_human plasma rifle";   });
	const EnemyGroup UnitFloodcombat_HumanRocketLauncher("Unit: Floodcombat_Human Rocket Launcher", "Floodcombat_Human Rocket Launcher", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat_human rocket launcher";   });
	const EnemyGroup UnitFloodcombat_HumanShotgun("Unit: Floodcombat_Human Shotgun", "Floodcombat_Human Shotgun", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat_human shotgun";   });
	const EnemyGroup UnitFloodcombat_HumanUnarmed("Unit: Floodcombat_Human Unarmed", "Floodcombat_Human Unarmed", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat_human unarmed";   });
	const EnemyGroup UnitFlood_Infection("Unit: Flood_Infection", "Flood_Infection", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "flood_infection";   });
	const EnemyGroup UnitGruntMajorNeedler("Unit: Grunt Major Needler", "Grunt Major Needler", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "grunt major needler";   });
	const EnemyGroup UnitGruntMajorPlasmaPistol("Unit: Grunt Major Plasma Pistol", "Grunt Major Plasma Pistol", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "grunt major plasma pistol";   });
	const EnemyGroup UnitGruntMinorNeedler("Unit: Grunt Minor Needler", "Grunt Minor Needler", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "grunt minor needler";   });
	const EnemyGroup UnitGruntMinorPlasmaPistol("Unit: Grunt Minor Plasma Pistol", "Grunt Minor Plasma Pistol", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "grunt minor plasma pistol";   });
	const EnemyGroup UnitGruntSpecopsFuelRod("Unit: Grunt Specops Fuel Rod", "Grunt Specops Fuel Rod", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "grunt specops fuel rod";   });
	const EnemyGroup UnitGruntSpecopsNeedler("Unit: Grunt Specops Needler", "Grunt Specops Needler", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "grunt specops needler";   });
	const EnemyGroup UnitHunterMajor("Unit: Hunter Major", "Hunter Major", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "hunter major";   });
	const EnemyGroup UnitHunter("Unit: Hunter", "Hunter", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "hunter";   });
	const EnemyGroup UnitJackalMajorPlasmaPistol("Unit: Jackal Major Plasma Pistol", "Jackal Major Plasma Pistol", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "jackal major plasma pistol";   });
	const EnemyGroup UnitJackalMinorPlasmaPistol("Unit: Jackal Minor Plasma Pistol", "Jackal Minor Plasma Pistol", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "jackal minor plasma pistol";   });
	const EnemyGroup UnitMarineAssaultRifleMajor("Unit: Marine Assault Rifle Major", "Marine Assault Rifle Major", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine assault rifle major";   });
	const EnemyGroup UnitMarineAssaultRifle("Unit: Marine Assault Rifle", "Marine Assault Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine assault rifle";   });
	const EnemyGroup UnitMarineNeedler("Unit: Marine Needler", "Marine Needler", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine needler";   });
	const EnemyGroup UnitMarinePlasmaRifle("Unit: Marine Plasma Rifle", "Marine Plasma Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine plasma rifle";   });
	const EnemyGroup UnitMarineShotgun("Unit: Marine Shotgun", "Marine Shotgun", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine shotgun";   });
	const EnemyGroup UnitMarine_ArmoredAssaultRifleMajor("Unit: Marine_Armored Assault Rifle Major", "Marine_Armored Assault Rifle Major", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine_armored assault rifle major";   });
	const EnemyGroup UnitMarine_ArmoredAssaultRifle("Unit: Marine_Armored Assault Rifle", "Marine_Armored Assault Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine_armored assault rifle";   });
	const EnemyGroup UnitMarine_ArmoredNeedler("Unit: Marine_Armored Needler", "Marine_Armored Needler", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine_armored needler";   });
	const EnemyGroup UnitMarine_ArmoredPlasmaRifleMajor("Unit: Marine_Armored Plasma Rifle Major", "Marine_Armored Plasma Rifle Major", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine_armored plasma rifle major";   });
	const EnemyGroup UnitMarine_ArmoredPlasmaRifle("Unit: Marine_Armored Plasma Rifle", "Marine_Armored Plasma Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine_armored plasma rifle";   });
	const EnemyGroup UnitMarine_ArmoredShotgunMajor("Unit: Marine_Armored Shotgun Major", "Marine_Armored Shotgun Major", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine_armored shotgun major";   });
	const EnemyGroup UnitMarine_ArmoredSniperRifleMajor("Unit: Marine_Armored Sniper Rifle Major", "Marine_Armored Sniper Rifle Major", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine_armored sniper rifle major";   });
	const EnemyGroup UnitMarine_ArmoredSniperRifle("Unit: Marine_Armored Sniper Rifle", "Marine_Armored Sniper Rifle", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "marine_armored sniper rifle";   });
	const EnemyGroup UnitSentinelMajor("Unit: Sentinel Major", "Sentinel Major", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "sentinel major";   });
	const EnemyGroup UnitSentinel("Unit: Sentinel", "Sentinel", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "sentinel";   });
	const EnemyGroup UnitSentinel_Defensive("Unit: Sentinel_Defensive", "Sentinel_Defensive", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "sentinel_defensive";   });
	const EnemyGroup UnitSentinel_ShieldedMajor("Unit: Sentinel_Shielded Major", "Sentinel_Shielded Major", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "sentinel_shielded major";   });
	const EnemyGroup UnitSentinel_Shielded("Unit: Sentinel_Shielded", "Sentinel_Shielded", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "sentinel_shielded";   });

#if includeFlamethrowerFloodOption == 1
	const EnemyGroup UnitFloodcombatEliteFlameThrower("Unit: Floodcombat Elite Flame Thrower", "Floodcombat Elite Flame Thrower", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat elite flame thrower";   });
	const EnemyGroup UnitFloodcombat_HumanFlameThrower("Unit: Floodcombat_Human Flame Thrower", "Floodcombat_Human Flame Thrower", [](UnitInfo& checkEnemy) {    return checkEnemy.getShortName() == "floodcombat_human flame thrower";   });
#endif

	const std::vector<EnemyGroup> builtInGroups
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
#if includeFlamethrowerFloodOption == 1
		UnitFloodcombatEliteFlameThrower,
#endif
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
		#if includeFlamethrowerFloodOption == 1
		UnitFloodcombat_HumanFlameThrower,
		#endif
		UnitFloodcombat_HumanNeedler,
		UnitFloodcombat_HumanPistol,
		UnitFloodcombat_HumanPlasmaPistol,
		UnitFloodcombat_HumanPlasmaRifle,
		UnitFloodcombat_HumanRocketLauncher,
		UnitFloodcombat_HumanShotgun,
		UnitFloodcombat_HumanUnarmed,
		UnitFlood_Infection,
		UnitGruntMajorNeedler,
		UnitGruntMajorPlasmaPistol,
		UnitGruntMinorNeedler,
		UnitGruntMinorPlasmaPistol,
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
