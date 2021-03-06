#include "tempentities.h"

using namespace LCViewer;

TempEntities::TempEntities(DocumentCanvas_SPtr docCanvas) : _docCanvas(docCanvas) {
}

void TempEntities::addEntity(lc::entity::CADEntity_CSPtr entity) {
    _entities.insert(entity);
}

void TempEntities::removeEntity(lc::entity::CADEntity_CSPtr entity) {
	_entities.remove(entity);
}

void TempEntities::onDraw(DrawEvent const &event) {
    _entities.each<const lc::entity::CADEntity>([&](lc::entity::CADEntity_CSPtr entity) {
        _docCanvas->drawEntity(event.painter(), _docCanvas->asDrawable(entity));
	});
}
