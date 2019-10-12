<template>
	<v-form>

		<v-row>
			<v-col>

				<s-input
					type="select"
					:data="'config.project.entities[0].animations.default'"
					label="Default"
					hint="Initial Animation"
					:items="optionsAnimations"
				></s-input>

			</v-col>
		</v-row>

		<v-row>
			<v-col>

				<div>Animation Definitions</div>
				<div>
					Total:
					{{ $root.data.data.config.project.entities[0].animations.definitions.length }} of
					{{ $root.data.data.config.project.config.animation.maxAnimationFunctions }} max
				</div>

			</v-col>
		</v-row>

		<v-row
			v-for="n, key in $root.data.data.config.project.entities[0].animations.definitions"
			:key="key"
		>
			<v-col>
				<v-card>
					<v-card-text>

						<v-row>
							<v-col>

								<s-input
									type="text"
									:data="'config.project.entities[0].animations.definitions[' + key + '].name'"
									label="Name"
									hint="Name of the animation"
									:maxlength="$root.data.data.config.project.config.animation.maxAnimationFunctionNameLength"
									:counter="$root.data.data.config.project.config.animation.maxAnimationFunctionNameLength"
								></s-input>

							</v-col>
						</v-row>
						<v-row>
							<v-col>

 								<!-- data type for cycles is s8, so 127 max -->
								<s-input
									type="number"
									:data="'config.project.entities[0].animations.definitions[' + key + '].cycles'"
									label="Cycles"
									hint="Cycles"
									min="1"
									max="127"
								></s-input>

							</v-col>
						</v-row>
						<v-row>
							<v-col>

								<s-input
									type="boolean"
									:data="'config.project.entities[0].animations.definitions[' + key + '].loop'"
									label="Loop"
									hint="Loop the animation of play only once"
								></s-input>

							</v-col>
							<v-col>

								<s-input v-if="!$root.data.data.config.project.entities[0].animations.definitions[key].loop"
									type="text"
									:data="'config.project.entities[0].animations.definitions[' + key + '].callback'"
									label="Callback"
									hint="Function to execute after the animation has finished"
								></s-input>
								<!-- TODO: else: hidden field -->

							</v-col>
						</v-row>
						<v-row>
							<v-col>

								<s-input
									type="text"
									:data="'config.project.entities[0].animations.definitions[' + key + '].frames'"
									label="Frames"
									hint="Frames to play in this animation"
								></s-input>

							</v-col>
						</v-row>

					</v-card-text>
				</v-card>
			</v-col>
		</v-row>

	</v-form>
</template>

<script>
	module.exports = {
		computed: {
			optionsAnimations() {
				let options = []
				for (let animation of this.$root.data.data.config.project.entities[0].animations.definitions) {
					options.push({
						text: animation.name,
						value: animation.name
					})
				}

				options.sort(function (a,b) {
					return a['text'].localeCompare(b['text']);
				});

				return options
			}
		}
	}
</script>

<style scoped>
</style>