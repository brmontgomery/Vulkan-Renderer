#pragma once

#include <string>

#include "PSIM/Core/Core.h"

class Texture
{
public:
	virtual ~Texture() = default;

	virtual const std::string& GetName() const = 0;
	virtual uint32_t GetWidth() const = 0;
	virtual uint32_t GetHeight() const = 0;
	virtual uint32_t GetRendererID() const = 0;
	virtual uint32_t GetMipLevels() const = 0;
	virtual void* GetImageView() const = 0;
	virtual void* GetSampler() const = 0;
	virtual uint32_t GetSlot() const = 0;
	virtual void destroy() const = 0;

	virtual void SetData(void* data, uint32_t size) = 0;

	virtual void Bind(uint32_t slot = 0) const = 0;

	virtual bool operator==(const Texture& other) const = 0;
};

class Texture2D : public Texture
{
public:
	static Ref<Texture2D> Create(const std::string& name, uint32_t width, uint32_t height);
	static Ref<Texture2D> Create(const std::string& path);
};

//-------------------------------------------------------------------------------------------

class TextureLibrary {
public:
	void Add(const Ref<Texture2D>& texure);
	void Add(const std::string& name, const Ref<Texture2D>& texture);

	Ref<Texture2D> Load(const std::string& name, uint32_t width, uint32_t height);
	Ref<Texture2D> Load(const std::string& path);

	Ref<Texture2D> Get(const std::string& name);

	bool exists(const std::string& name) const;
	void remove(const std::string& name);

	std::vector<std::string>* getBoundTextures() { return &boundTextures; }
	inline void bindTexture(std::string name) { if (exists(name)) { boundTextures.push_back(name); } }
	void resetBoundTextures() { boundTextures.clear(); }

	void cleanUp();

private:
	std::unordered_map<std::string, Ref<Texture2D>> m_Textures;
	std::vector<std::string> boundTextures;
};