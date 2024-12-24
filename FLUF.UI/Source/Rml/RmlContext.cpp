#include "PCH.hpp"

#include "Rml/RmlContext.hpp"

#include "Fluf.hpp"

Rml::ElementDocument* RmlContext::LoadDocument(const Rml::String& filePath) const
{
    Fluf::Log(LogLevel::Debug, std::format("Attempting to load document: {}", filePath));
    return context->LoadDocument(filePath);
}

Rml::ElementDocument* RmlContext::LoadDocumentFromMemory(const Rml::String& documentContents) const
{
    return context->LoadDocumentFromMemory(documentContents);
}

void RmlContext::UnloadDocument(Rml::ElementDocument* document) const { context->UnloadDocument(document); }

void RmlContext::UnloadAllDocuments() const { context->UnloadAllDocuments(); }

void RmlContext::UnfocusDocument(Rml::ElementDocument* document) const { context->UnfocusDocument(document); }

void RmlContext::UnfocusAllDocuments() const { context->UnloadAllDocuments(); }

bool RmlContext::RemoveDataModel(const Rml::String& name) const { return context->RemoveDataModel(name); }

Rml::DataModelConstructor RmlContext::GetDataModel(const Rml::String& name) const { return context->GetDataModel(name); }

Rml::DataModelConstructor RmlContext::CreateDataModel(const Rml::String& name, Rml::DataTypeRegister* dataTypeRegister) const
{
    return context->CreateDataModel(name, dataTypeRegister);
}

RmlContext::RmlContext(Rml::Context* context) : context(context) {}
