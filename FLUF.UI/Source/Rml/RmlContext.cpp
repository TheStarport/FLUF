#include "PCH.hpp"

#include "Rml/RmlContext.hpp"

Rml::ElementDocument* RmlContext::LoadDocument(const Rml::String& filePath) { return context->LoadDocument(filePath); }

Rml::ElementDocument* RmlContext::LoadDocumentFromMemory(const Rml::String& documentContents) { return context->LoadDocumentFromMemory(documentContents); }

void RmlContext::UnloadDocument(Rml::ElementDocument* document) { context->UnloadDocument(document); }

void RmlContext::UnloadAllDocuments() { context->UnloadAllDocuments(); }

void RmlContext::UnfocusDocument(Rml::ElementDocument* document) { context->UnfocusDocument(document); }

void RmlContext::UnfocusAllDocuments() { context->UnloadAllDocuments(); }

bool RmlContext::RemoveDataModel(const Rml::String& name) { return context->RemoveDataModel(name); }

Rml::DataModelConstructor RmlContext::GetDataModel(const Rml::String& name) { return context->GetDataModel(name); }

Rml::DataModelConstructor RmlContext::CreateDataModel(const Rml::String& name, Rml::DataTypeRegister* dataTypeRegister)
{
    return context->CreateDataModel(name, dataTypeRegister);
}

RmlContext::RmlContext(Rml::Context* context) : context(context) {}
