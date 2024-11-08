#pragma once

#include "ImportFlufUi.hpp"

#include <RmlUi/Core.h>

class RmlInterface;
class FlufUi;

/**
 * @brief A very basic wrapper around the RmlContext containing only the useful functions.
 * The original context can be accessed through the Rml::ElementDocument pointer.
 */
class FLUF_UI_API RmlContext final
{
        friend RmlInterface;
        friend FlufUi;

        Rml::Context* context;

        RmlContext(Rml::Context*);

    public:
        /**
         * @brief Loads the RML document at the specified location and returns a pointer to the loaded document.
         * All newly documents are hidden by default and must be manually shown.
         * @param filePath A file path pointing to a .rml file. The path must begin with a protocol being either 'file' or 'local'.
         * If the protocol is 'file://' then the file will be relative to the EXE folder of Freelancer, or be an absolute path.
         * If the protocol is 'local://' then the file must be relative to the DATA folder
         * @returns A pointer to an ElementDocument on success, or a nullptr if there was an error loading it.
         * @example LoadDocument("local://INTERFACE/RML/document.rml")
         */
        Rml::ElementDocument* LoadDocument(const Rml::String& filePath) const;

        /**
         *
         * @param documentContents A string containing a valid RML document
         * @returns A pointer to an ElementDocument on success, or a nullptr if there was an error loading it.
         */
        Rml::ElementDocument* LoadDocumentFromMemory(const Rml::String& documentContents) const;

        /**
         * @brief Unloads the specified document.
         * @param document A pointer to the document to unload
         */
        void UnloadDocument(Rml::ElementDocument* document) const;

        /**
         * @brief Unloads all loaded documents
         */
        void UnloadAllDocuments() const;

        /**
         * @brief If a specific document has focus this function can be used to remove focus from it programmatically.
         */
        void UnfocusDocument(Rml::ElementDocument* document) const;

        /**
         * @brief Removes focus from all documents effectively removing focus from RML itself until a document is clicked again.
         */
        void UnfocusAllDocuments() const;

        /**
         * Creates a data model.
         * @brief The returned constructor can be used to bind data variables. Elements can bind to the model using the attribute 'data-model="name"'.
         * @param[in] name The name of the data model.
         * @param[in] dataTypeRegister The data type register to use for the data model, or null to use the default register.
         * @return A constructor for the data model, or empty if it could not be created.
         */
        Rml::DataModelConstructor CreateDataModel(const Rml::String& name, Rml::DataTypeRegister* dataTypeRegister = nullptr) const;

        /**
         * Retrieves the constructor for an existing data model.
         * @brief The returned constructor can be used to add additional bindings to an existing model.
         * @param[in] name The name of the data model.
         * @returns A constructor for the data model, or empty if it could not be found.
         */
        Rml::DataModelConstructor GetDataModel(const Rml::String& name) const;

        /** Removes the given data model.
         * @brief This also removes all data views, controllers and bindings contained by the data model.
         * @warning Invalidates all handles and constructors pointing to the data model.
         * @param[in] name The name of the data model.
         * @return True if successfully removed, false if no data model was found.
         */
        bool RemoveDataModel(const Rml::String& name) const;
};
