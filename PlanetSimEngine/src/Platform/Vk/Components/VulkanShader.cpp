#include "PSIMPCH.h"
/*#include "Platform/Vk/Components/VulkanShader.h"

#include <fstream>
#include <vulkan/vulkan.hpp>

#include <glm/gtc/type_ptr.hpp>

static vk::ShaderStageFlags ShaderTypeFromString(const std::string& type)
{
	if (type == "vertex")
		return vk::ShaderStageFlagBits::eVertex;
	if (type == "fragment" || type == "pixel")
		return vk::ShaderStageFlagBits::eFragment;

	PSIM_ASSERT(false, "Unknown shader type!");
	return vk::ShaderStageFlagBits::eAll;
}

VulkanShader::VulkanShader(const std::string& filepath)
{
	PSIM_PROFILE_FUNCTION();

	framework = VulkanFrameWork::getFramework();

	std::string source = ReadFile(filepath);
	auto shaderSources = PreProcess(source);
	Compile(shaderSources);

	// Extract name from filepath
	auto lastSlash = filepath.find_last_of("/\\");
	lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
	auto lastDot = filepath.rfind('.');
	auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
	m_Name = filepath.substr(lastSlash, count);
}

VulkanShader::VulkanShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	: m_Name(name)
{
	PSIM_PROFILE_FUNCTION();

	framework = VulkanFrameWork::getFramework();

	std::unordered_map<vk::ShaderStageFlags, std::string> sources;
	sources[vk::ShaderStageFlagBits::eVertex] = vertexSrc;
	sources[vk::ShaderStageFlagBits::eFragment] = fragmentSrc;
	Compile(sources);
}

VulkanShader::~VulkanShader()
{
	PSIM_PROFILE_FUNCTION();
}

void VulkanShader::deleteShader()
{
	if (bool(vertexShader)) {
		vkDestroyShaderModule(framework->getDevice(), vertexShader, nullptr);
	}
	if (bool(fragmentShader)) {
		vkDestroyShaderModule(framework->getDevice(), fragmentShader, nullptr);
	}
}

std::string VulkanShader::ReadFile(const std::string& filepath)
{
	PSIM_PROFILE_FUNCTION();

	std::string result;
	std::ifstream in(filepath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
	if (in)
	{
		in.seekg(0, std::ios::end);
		size_t size = in.tellg();
		if (size != -1)
		{
			result.resize(size);
			in.seekg(0, std::ios::beg);
			in.read(&result[0], size);
		}
		else
		{
			PSIM_CORE_ERROR("Could not read from file '{0}'", filepath);
		}
	}
	else
	{
		PSIM_CORE_ERROR("Could not open file '{0}'", filepath);
	}

	return result;
}

std::unordered_map<vk::ShaderStageFlags, std::string> VulkanShader::PreProcess(const std::string& source)
{
	PSIM_PROFILE_FUNCTION();

	std::unordered_map<vk::ShaderStageFlags, std::string> shaderSources;

	const char* typeToken = "#type";
	size_t typeTokenLength = strlen(typeToken);
	size_t pos = source.find(typeToken, 0); //Start of shader type declaration line
	while (pos != std::string::npos)
	{
		size_t eol = source.find_first_of("\r\n", pos); //End of shader type declaration line
		PSIM_ASSERT(eol != std::string::npos, "Syntax error");
		size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
		std::string type = source.substr(begin, eol - begin);
		PSIM_ASSERT(ShaderTypeFromString(type), "Invalid shader type specified");

		size_t nextLinePos = source.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
		PSIM_ASSERT(nextLinePos != std::string::npos, "Syntax error");
		pos = source.find(typeToken, nextLinePos); //Start of next shader type declaration line

		shaderSources[ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
	}

	return shaderSources;
}

void VulkanShader::Compile(const std::unordered_map<vk::ShaderStageFlags, std::string>& shaderSources)
{
	PSIM_PROFILE_FUNCTION();

	GLuint program = glCreateProgram();
	PSIM_ASSERT(shaderSources.size() <= 2, "We only support 2 shaders for now");
	std::array<GLenum, 2> glShaderIDs;
	int glShaderIDIndex = 0;
	for (auto& kv : shaderSources)
	{
		GLenum type = kv.first;
		const std::string& source = kv.second;

		GLuint shader = glCreateShader(type);

		const GLchar* sourceCStr = source.c_str();
		glShaderSource(shader, 1, &sourceCStr, 0);

		glCompileShader(shader);

		GLint isCompiled = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

			glDeleteShader(shader);

			PSIM_CORE_ERROR("{0}", infoLog.data());
			PSIM_ASSERT(false, "Shader compilation failure!");
			break;
		}

		glAttachShader(program, shader);
		glShaderIDs[glShaderIDIndex++] = shader;
	}

	m_RendererID = program;

	// Link our program
	glLinkProgram(program);

	// Note the different functions here: glGetProgram* instead of glGetShader*.
	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

		// We don't need the program anymore.
		glDeleteProgram(program);

		for (auto id : glShaderIDs)
			glDeleteShader(id);

		PSIM_CORE_ERROR("{0}", infoLog.data());
		PSIM_ASSERT(false, "Shader link failure!");
		return;
	}

	for (auto id : glShaderIDs)
	{
		glDetachShader(program, id);
		glDeleteShader(id);
	}
}

void VulkanShader::Bind() const
{
	PSIM_PROFILE_FUNCTION();

	glUseProgram(m_RendererID);
}

void VulkanShader::Unbind() const
{
	PSIM_PROFILE_FUNCTION();

	glUseProgram(0);
}



































inline bool IsSPIR_V(const std::string_view &fileName)
{
	const std::string_view extention = fileName.substr(fileName.size() - 4, 4);
	return extention == ".spv";
}

inline shaderc_shader_kind InferKindFromFileName(const std::string_view &fileName)
{
	// Get file extension
	const auto dotIndex = fileName.find_last_of('.');
	const auto extension = fileName.substr(dotIndex + 1);

	// Fall-back in case we cannot infer type from extension
	shaderc_shader_kind kind = shaderc_shader_kind::shaderc_glsl_infer_from_source;

	if (extension == "comp") {
		kind = shaderc_shader_kind::shaderc_glsl_default_compute_shader;
	}
	else if (extension == "vert") {
		kind = shaderc_shader_kind::shaderc_glsl_default_vertex_shader;
	}
	else if (extension == "frag") {
		kind = shaderc_shader_kind::shaderc_glsl_default_fragment_shader;
	}
	else if (extension == "geom") {
		kind = shaderc_shader_kind::shaderc_glsl_default_geometry_shader;
	}
	else if (extension == "rchit") {
		//kind = shaderc_shader_kind::shaderc_glsl_default_closesthit_shader;
	}
	else if (extension == "rgen") {
		//kind = shaderc_shader_kind::shaderc_glsl_default_raygen_shader;
	}
	else if (extension == "rmiss") {
		//kind = shaderc_shader_kind::shaderc_glsl_default_miss_shader;
	}
	else if (extension == "rahit") {
		//kind = shaderc_shader_kind::shaderc_glsl_default_anyhit_shader;
	}
	else if (extension == "mesh") {
		//kind = shaderc_shader_kind::shaderc_glsl_default_mesh_shader;
	}
	else if (extension == "rcall") {
		//kind = shaderc_shader_kind::shaderc_glsl_default_callable_shader;
	}

	return kind;
}

VulkanShader::VulkanShader(const std::string_view &fileName,
	const std::vector<std::pair<std::string, std::string>> &definitions)
{
	const auto cwd = get_cwd();
	const std::string fileLocation = cwd + "/" + BaseFolder + std::string(fileName.data());

	// Sanity check
	if (!FileExists(fileLocation.c_str()))
		PSIM_CORE_ERROR("File: \"{0}\" does not exist.", fileLocation.data());

	// Setup compiler environment
	m_CompileOptions.SetIncluder(std::make_unique<FileIncluder>(&m_Finder));
	m_CompileOptions.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_1);
	m_CompileOptions.SetSourceLanguage(shaderc_source_language_glsl);
	m_CompileOptions.SetOptimizationLevel(shaderc_optimization_level_performance);
	m_Finder.search_path().push_back(BSDFFolder);

	std::vector<std::pair<std::string, std::string>> defs = definitions;
	defs.push_back(std::make_pair("__VK_GLSL__", "1"));

	for (auto &defPair : defs) // Add given definitions to compiler
		m_CompileOptions.AddMacroDefinition(defPair.first, defPair.second);

	if (IsSPIR_V(fileName)) // In case we get fed an pre-compiled SPIR-V shader
	{
		const auto source = ReadFile(fileLocation);
		vk::ShaderModuleCreateInfo shaderModuleCreateInfo = vk::ShaderModuleCreateInfo(vk::ShaderModuleCreateFlags(), source.size(), (uint32_t *)(source.data()));
		m_Module = m_Device->createShaderModule(shaderModuleCreateInfo);
		PSIM_ASSERT(!m_Module, "Could not create shader module for shader: \"{0}\".", fileLocation.data());
	}
	else // We need to compile the shader ourselves
	{
		const std::string sourceString = ReadTextFile(fileLocation);			  // Get source of shader
		const auto kind = InferKindFromFileName(fileLocation);				  // Infer shader kind
		const auto result = PreprocessShader(fileLocation, sourceString, kind); // Preprocess source file
		const auto binary = CompileFile(fileLocation, result, kind);			  // Produce SPIR-V binary

		m_Module = m_Device->createShaderModule(vk::ShaderModuleCreateInfo({}, binary.size() * sizeof(uint32_t), binary.data()));
		PSIM_ASSERT(!m_Module, "Could not create shader module for shader: \"{0}\".", fileLocation.data());
	}
}

VulkanShader::~VulkanShader()
{
	Cleanup();
}

void VulkanShader::Cleanup()
{
	if (m_Module)
	{
		m_Device->destroyShaderModule(m_Module);
		m_Module = nullptr;
	}
}

vk::PipelineShaderStageCreateInfo VulkanShader::GetShaderStage(vk::ShaderStageFlagBits stage) const
{
	vk::PipelineShaderStageCreateInfo result{};
	result.setPNext(nullptr);
	result.setStage(stage);
	result.setModule(m_Module);
	result.setPName("main");
	result.setFlags(vk::PipelineShaderStageCreateFlags());
	result.setPSpecializationInfo(nullptr);
	return result;
}

std::vector<char> VulkanShader::ReadFile(const std::string_view &fileName)
{
	std::ifstream fileStream(fileName.data(), std::ios::binary | std::ios::in | std::ios::ate);
	if (!fileStream.is_open())
		PSIM_CORE_ERROR("Could not open file.");

	const size_t size = fileStream.tellg();
	fileStream.seekg(0, std::ios::beg);
	std::vector<char> data(size);
	fileStream.read(data.data(), size);
	fileStream.close();
	return data;
}

std::string VulkanShader::ReadTextFile(const std::string_view &fileName)
{
	std::string buffer;
	std::ifstream fileStream(fileName.data());
	if (!fileStream.is_open())
		PSIM_CORE_ERROR("Could not open file.");

	std::string temp;
	while (getline(fileStream, temp))
		buffer.append(temp), buffer.append("\n");

	fileStream >> buffer;
	fileStream.close();
	return buffer;
}

//-----------------------------------------------------------------------------------------------------
//
//ShaderC Funcs
//
//-----------------------------------------------------------------------------------------------------

// The shader compilation implementation below was taken from google/shaderc itself
// It implements include support for GLSL
// https://github.com/google/shaderc
std::string VulkanShader::PreprocessShader(const std::string_view &fileName, const std::string &source, shaderc_shader_kind shaderKind)
{
	auto result = m_Compiler.PreprocessGlsl(source, shaderKind, fileName.data(), m_CompileOptions);
	if (result.GetCompilationStatus() != shaderc_compilation_status_success)
	{
		PSIM_CORE_ERROR("{0}", result.GetErrorMessage().data());
		return "";
	}
	return { result.cbegin(), result.cend() };
}

std::string VulkanShader::CompileToAssembly(const std::string_view &fileName, const std::string &source, shaderc_shader_kind shaderKind)
{
	auto result = m_Compiler.CompileGlslToSpvAssembly(source, shaderKind, fileName.data(), m_CompileOptions);

	if (result.GetCompilationStatus() != shaderc_compilation_status_success)
	{
		PSIM_CORE_ERROR("{0}", result.GetErrorMessage().data());
		return "";
	}
	return { result.cbegin(), result.cend() };
}

std::vector<uint32_t> VulkanShader::CompileFileToSPV(const std::string_view &fileName, const std::string &source, shaderc_shader_kind shaderKind)
{
	auto module = m_Compiler.CompileGlslToSpv(source, shaderKind, fileName.data(), m_CompileOptions);

	if (module.GetCompilationStatus() != shaderc_compilation_status_success)
	{
		PSIM_CORE_ERROR("{0}", module.GetErrorMessage().data());
		return std::vector<uint32_t>();
	}

	return { module.cbegin(), module.cend() };
}

shaderc_include_result *MakeErrorIncludeResult(const char *message)
{
	return new shaderc_include_result{ "", 0, message, strlen(message) };
}

//-----------------------------------------------------------------------------------------------------
//
//Shader File Finding and Editing
//
//-----------------------------------------------------------------------------------------------------

class string_piece
{
public:
	typedef const char *iterator;
	static const size_t npos = -1;

	string_piece() {}

	string_piece(const char *begin, const char *end) : begin_(begin), end_(end)
	{
		PSIM_ASSERT((begin == nullptr) == (end == nullptr), "Either both begin and end must be nullptr or neither must be!");
	}

	string_piece(const char *string) : begin_(string), end_(string)
	{
		if (string)
		{
			end_ += strlen(string);
		}
	}

	string_piece(const std::string &str)
	{
		if (!str.empty())
		{
			begin_ = &(str.front());
			end_ = &(str.back()) + 1;
		}
	}

	string_piece(const string_piece &other)
	{
		begin_ = other.begin_;
		end_ = other.end_;
	}

	void clear()
	{
		begin_ = nullptr;
		end_ = nullptr;
	}

	const char *data() const { return begin_; }

	std::string str() const { return std::string(begin_, end_); }

	string_piece substr(size_t pos, size_t len = npos) const
	{
		PSIM_ASSERT(len == npos || pos + len <= size(), "Length is incorrect!");
		return string_piece(begin_ + pos, len == npos ? end_ : begin_ + pos + len);
	}

	template <typename T>
	size_t find_first_not_matching(T callee)
	{
		for (auto it = begin_; it != end_; ++it)
		{
			if (!callee(*it))
			{
				return it - begin_;
			}
		}
		return npos;
	}

	size_t find_first_not_of(const string_piece &to_search,
		size_t pos = 0) const
	{
		if (pos >= size())
		{
			return npos;
		}
		for (auto it = begin_ + pos; it != end_; ++it)
		{
			if (to_search.find_first_of(*it) == npos)
			{
				return it - begin_;
			}
		}
		return npos;
	}

	size_t find_first_not_of(char to_search, size_t pos = 0) const
	{
		return find_first_not_of(string_piece(&to_search, &to_search + 1), pos);
	}

	size_t find_first_of(const string_piece &to_search, size_t pos = 0) const
	{
		if (pos >= size())
		{
			return npos;
		}
		for (auto it = begin_ + pos; it != end_; ++it)
		{
			for (char c : to_search)
			{
				if (c == *it)
				{
					return it - begin_;
				}
			}
		}
		return npos;
	}

	size_t find_first_of(char to_search, size_t pos = 0) const
	{
		return find_first_of(string_piece(&to_search, &to_search + 1), pos);
	}

	size_t find_last_of(const string_piece &to_search, size_t pos = npos) const
	{
		if (empty()) return npos;
		if (pos >= size())
		{
			pos = size();
		}
		auto it = begin_ + pos + 1;
		do
		{
			--it;
			if (to_search.find_first_of(*it) != npos)
			{
				return it - begin_;
			}
		} while (it != begin_);
		return npos;
	}

	size_t find_last_of(char to_search, size_t pos = npos) const
	{
		return find_last_of(string_piece(&to_search, &to_search + 1), pos);
	}

	size_t find_last_not_of(const string_piece &to_search,
		size_t pos = npos) const
	{
		if (empty()) return npos;
		if (pos >= size())
		{
			pos = size();
		}
		auto it = begin_ + pos + 1;
		do
		{
			--it;
			if (to_search.find_first_of(*it) == npos)
			{
				return it - begin_;
			}
		} while (it != begin_);
		return npos;
	}

	size_t find_last_not_of(char to_search, size_t pos = 0) const
	{
		return find_last_not_of(string_piece(&to_search, &to_search + 1), pos);
	}

	string_piece lstrip(const string_piece &chars_to_strip) const
	{
		iterator begin = begin_;
		for (; begin < end_; ++begin)
			if (chars_to_strip.find_first_of(*begin) == npos) break;
		if (begin >= end_) return string_piece();
		return string_piece(begin, end_);
	}

	string_piece rstrip(const string_piece &chars_to_strip) const
	{
		iterator end = end_;
		for (; begin_ < end; --end)
			if (chars_to_strip.find_first_of(*(end - 1)) == npos) break;
		if (begin_ >= end) return string_piece();
		return string_piece(begin_, end);
	}

	string_piece strip(const string_piece &chars_to_strip) const { return lstrip(chars_to_strip).rstrip(chars_to_strip); }
	string_piece strip_whitespace() const { return strip(" \t\n\r\f\v"); }
	const char &operator[](size_t i) const { return *(begin_ + i); }
	bool operator==(const string_piece &other) const
	{
		// Either end_ and _begin_ are nullptr or neither of them are.
		PSIM_ASSERT((end_ == nullptr) == (begin_ == nullptr), "String piece is empty!");
		PSIM_ASSERT(((other.end_ == nullptr) == (other.begin_ == nullptr)), "String piece is empty!");
		if (size() != other.size())
		{
			return false;
		}
		return (memcmp(begin_, other.begin_, end_ - begin_) == 0);
	}

	bool operator!=(const string_piece &other) const
	{
		return !operator==(other);
	}

	iterator begin() const { return begin_; }
	iterator end() const { return end_; }

	const char &front() const
	{
		PSIM_ASSERT(!empty(), "String piece is empty!");
		return *begin_;
	}

	const char &back() const
	{
		PSIM_ASSERT(!empty(), "String piece is empty!");
		return *(end_ - 1);
	}

	bool starts_with(const string_piece &other) const
	{
		const char *iter = begin_;
		const char *other_iter = other.begin();
		while (iter != end_ && other_iter != other.end())
		{
			if (*iter++ != *other_iter++)
			{
				return false;
			}
		}
		return other_iter == other.end();
	}

	size_t find(const string_piece &substr, size_t pos = 0) const
	{
		if (empty()) return npos;
		if (pos >= size()) return npos;
		if (substr.empty()) return 0;
		for (auto it = begin_ + pos;
			end() - it >= static_cast<decltype(end() - it)>(substr.size()); ++it)
		{
			if (string_piece(it, end()).starts_with(substr)) return it - begin_;
		}
		return npos;
	}
	size_t find(char character, size_t pos = 0) const { return find_first_of(character, pos); }
	bool empty() const { return begin_ == end_; }
	size_t size() const { return end_ - begin_; }
	std::vector<string_piece> get_fields(char delimiter,
		bool keep_delimiter = false) const
	{
		std::vector<string_piece> fields;
		size_t first = 0;
		size_t field_break = find_first_of(delimiter);
		while (field_break != npos)
		{
			fields.push_back(substr(first, field_break - first + keep_delimiter));
			first = field_break + 1;
			field_break = find_first_of(delimiter, first);
		}
		if (size() - first > 0)
		{
			fields.push_back(substr(first, size() - first));
		}
		return fields;
	}

	friend std::ostream &operator<<(std::ostream &os, const string_piece &piece);

	private:
	  string_piece::iterator begin_ = nullptr;
	  string_piece::iterator end_ = nullptr;
};

inline std::ostream &operator<<(std::ostream &os, const string_piece &piece)
{
	// Either end_ and _begin_ are nullptr or neither of them are.
	PSIM_ASSERT(((piece.end_ == nullptr) == (piece.begin_ == nullptr)), "Piece is empty!");
	if (piece.end_ != piece.begin_)
	{
		os.write(piece.begin_, piece.end_ - piece.begin_);
	}
	return os;
}

inline bool operator==(const char *first, const string_piece second)
{
	return second == first;
}

inline bool operator!=(const char *first, const string_piece second)
{
	return !operator==(first, second);
}

// Returns "" if path is empty or ends in '/'.  Otherwise, returns "/".
std::string MaybeSlash(const string_piece &path)
{
	return (path.empty() || path.back() == '/') ? "" : "/";
}

//-----------------------------------------------------------------------------------------------------
//
//File Finder
//
//-----------------------------------------------------------------------------------------------------

std::string VulkanShader::FileFinder::FindReadableFilepath(
	const std::string &filename) const
{
	PSIM_ASSERT(!filename.empty(), "Filename is empty!");
	static const auto for_reading = std::ios_base::in;
	std::filebuf opener;
	for (const auto &prefix : search_path_)
	{
		const std::string prefixed_filename =
			prefix + MaybeSlash(prefix) + filename;
		if (opener.open(prefixed_filename, for_reading)) return prefixed_filename;
	}
	return "";
}

std::string VulkanShader::FileFinder::FindRelativeReadableFilepath(
	const std::string &requesting_file, const std::string &filename) const
{
	PSIM_ASSERT(!filename.empty(), "Filename is empty!");

	string_piece dir_name(requesting_file);

	size_t last_slash = requesting_file.find_last_of("/\\");
	if (last_slash != std::string::npos)
	{
		dir_name = string_piece(requesting_file.c_str(),
			requesting_file.c_str() + last_slash);
	}

	if (dir_name.size() == requesting_file.size())
	{
		dir_name.clear();
	}

	static const auto for_reading = std::ios_base::in;
	std::filebuf opener;
	const std::string relative_filename =
		dir_name.str() + MaybeSlash(dir_name) + filename;
	if (opener.open(relative_filename, for_reading)) return relative_filename;

	return FindReadableFilepath(filename);
}

VulkanShader::FileIncluder::~FileIncluder() = default;

shaderc_include_result *VulkanShader::FileIncluder::GetInclude(
	const char *requested_source, shaderc_include_type include_type,
	const char *requesting_source, size_t)
{

	const std::string full_path =
		(include_type == shaderc_include_type_relative)
		? file_finder_.FindRelativeReadableFilepath(requesting_source,
			requested_source)
		: file_finder_.FindReadableFilepath(requested_source);

	if (full_path.empty())
		return MakeErrorIncludeResult("Cannot find or open include file.");

	// In principle, several threads could be resolving includes at the same
	// time.  Protect the included_files.

	const auto ReadFile = [](const std::string &input_file_name,
		std::vector<char> *input_data) -> bool {
		std::istream *stream = &std::cin;
		std::ifstream input_file;
		if (input_file_name != "-")
		{
			input_file.open(input_file_name, std::ios_base::binary);
			stream = &input_file;
			if (input_file.fail())
			{
				std::string errorMessage = std::string("Cannot open input file: ") + input_file_name;
				PSIM_CORE_ERROR("{0}", errorMessage.data());
				return false;
			}
		}
		*input_data = std::vector<char>((std::istreambuf_iterator<char>(*stream)),
			std::istreambuf_iterator<char>());
		return true;
	};

	// Read the file and save its full path and contents into stable addresses.
	FileInfo *new_file_info = new FileInfo{ full_path, {} };
	if (!ReadFile(full_path, &(new_file_info->contents)))
	{
		return MakeErrorIncludeResult("Cannot read file");
	}

	included_files_.insert(full_path);

	return new shaderc_include_result{
		new_file_info->full_path.data(), new_file_info->full_path.length(),
		new_file_info->contents.data(), new_file_info->contents.size(),
		new_file_info };
}

void VulkanShader::FileIncluder::ReleaseInclude(shaderc_include_result *include_result)
{
	FileInfo *info = static_cast<FileInfo *>(include_result->user_data);
	delete info;
	delete include_result;
}*/