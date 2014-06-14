
Overlord avoid behaivor:

	TilePosition tpos = unit->getTilePosition();
	if (tm->isThreatPosition(tpos, false)) {
		unit->rightClick(tm->getGreedyFleePosition(tpos, false));
	}
	
Zergling scout behaivor:

	TilePosition tpos = unit->getTilePosition();
	if (tm->isThreatPosition(tpos, true)) {
		unit->rightClick(tm->getGreedyFleePosition(tpos, true));
	} else if (tm->isEdgePosition(tpos, true)) {
		newpos = tm->getNextEdgePosition(tpos, true, clockwise);
		if (newpos != TilePosition::Unknown) {
			unit->rightClick(newpos);
		} else {
			clockwise = !clockwise;
			unit->rightClick(currentjob.tileposition);
		}
	}