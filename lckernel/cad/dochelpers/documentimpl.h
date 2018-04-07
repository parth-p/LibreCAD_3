#pragma once

#include <mutex>
#include <vector>
#include <string>
#include <unordered_set>
#include <map>
#include "cad/const.h"

#include "cad/document/document.h"
#include "cad/document/storagemanager.h"
#include "cad/operations/documentoperation.h"

namespace lc {
    class DocumentImpl : public Document {
        public:
            DocumentImpl(StorageManager_SPtr storageManager);
            virtual ~DocumentImpl();

        public:
            void insertEntity(entity::CADEntity_CSPtr cadEntity) override;
            void removeEntity(entity::CADEntity_CSPtr entity) override;

            void addDocumentMetaType(DocumentMetaType_CSPtr dmt) override;
            void removeDocumentMetaType(DocumentMetaType_CSPtr dmt) override;
            void replaceDocumentMetaType(DocumentMetaType_CSPtr oldDmt, DocumentMetaType_CSPtr newDmt) override;
            std::map<std::string, DocumentMetaType_CSPtr, lc::StringHelper::cmpCaseInsensetive> allMetaTypes() override;
            /**
             * @brief entitiesByLayer
             * @param layer
             * @return
             */
            EntityContainer<entity::CADEntity_CSPtr> entitiesByLayer(Layer_CSPtr layer) override;

            EntityContainer<entity::CADEntity_CSPtr> entitiesByBlock(Block_CSPtr block) override;

            EntityContainer<entity::CADEntity_CSPtr> entityContainer() override;

            std::map<std::string, Layer_CSPtr> allLayers() const override;

            Layer_CSPtr layerByName(const std::string& layerName) const override;
            DxfLinePatternByValue_CSPtr linePatternByName(const std::string& linePatternName) const override;

            /**
             * @brief return all line patterns
             */
            std::vector<DxfLinePatternByValue_CSPtr> linePatterns() const override;

        protected:
            /*!
             * \brief execute's a operation
             * \param operation
             */
            void execute(operation::DocumentOperation_SPtr operation) override;

            /*!
             * \brief begins operation
             * \param operation
             */
            void begin(operation::DocumentOperation_SPtr operation) override;
            /*!
             * \brief commits operation
             * \param operation
             */
            void commit(operation::DocumentOperation_SPtr operation) override;

            StorageManager_SPtr storageManager() const;

        public:
            std::unordered_set<entity::Insert_CSPtr> waitingCustomEntities(const std::string& pluginName) override;

            std::vector<Block_CSPtr> blocks() const override;

        private:
            std::mutex _documentMutex;
            // AI am considering remove the shared_ptr from this one so we can never get a shared object from it
            StorageManager_SPtr _storageManager;

            std::map<std::string, std::unordered_set<entity::Insert_CSPtr>> _waitingCustomEntities;
            std::unordered_set<entity::Insert_CSPtr> _newWaitingCustomEntities;
    };
}
